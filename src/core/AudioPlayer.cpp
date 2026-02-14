#include "AudioPlayer.h"
#include <QDebug>

AudioPlayer::AudioPlayer(QObject *parent)
    : QIODevice(parent)
{
}

AudioPlayer::~AudioPlayer()
{
    stop();
    close();
    if (m_packet) av_packet_free(&m_packet);
    if (m_frame) av_frame_free(&m_frame);
    if (m_resampledFrame) av_frame_free(&m_resampledFrame);
}

bool AudioPlayer::openFile(const QString &filePath)
{
    close(); // 确保先彻底关闭之前的资源
    QMutexLocker locker(&m_mutex);
    
    m_formatCtx = avformat_alloc_context();
    if (avformat_open_input(&m_formatCtx, filePath.toUtf8().constData(), nullptr, nullptr) != 0) {
        qDebug() << "Failed to open input file:" << filePath;
        return false;
    }

    if (avformat_find_stream_info(m_formatCtx, nullptr) < 0) {
        qDebug() << "Failed to find stream info";
        avformat_close_input(&m_formatCtx);
        m_formatCtx = nullptr;
        return false;
    }

    m_audioStreamIndex = -1;
    for (unsigned int i = 0; i < m_formatCtx->nb_streams; i++) {
        if (m_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            m_audioStreamIndex = i;
            break;
        }
    }

    if (m_audioStreamIndex == -1) {
        qDebug() << "No audio stream found";
        avformat_close_input(&m_formatCtx);
        m_formatCtx = nullptr;
        return false;
    }

    AVCodecParameters *codecParams = m_formatCtx->streams[m_audioStreamIndex]->codecpar;
    const AVCodec *codec = avcodec_find_decoder(codecParams->codec_id);
    if (!codec) {
        qDebug() << "Decoder not found";
        avformat_close_input(&m_formatCtx);
        m_formatCtx = nullptr;
        return false;
    }

    m_codecCtx = avcodec_alloc_context3(codec);
    if (!m_codecCtx) {
        avformat_close_input(&m_formatCtx);
        m_formatCtx = nullptr;
        return false;
    }

    avcodec_parameters_to_context(m_codecCtx, codecParams);

    if (avcodec_open2(m_codecCtx, codec, nullptr) < 0) {
        qDebug() << "Failed to open codec";
        avcodec_free_context(&m_codecCtx);
        m_codecCtx = nullptr;
        avformat_close_input(&m_formatCtx);
        m_formatCtx = nullptr;
        return false;
    }

    setupResampler();
    if (!m_swrCtx) {
        qDebug() << "Failed to setup resampler";
        avcodec_free_context(&m_codecCtx);
        m_codecCtx = nullptr;
        avformat_close_input(&m_formatCtx);
        m_formatCtx = nullptr;
        return false;
    }

    if (m_formatCtx->duration != AV_NOPTS_VALUE) {
        m_duration = m_formatCtx->duration / 1000;
    }

    m_position = 0;
    m_buffer.clear();
    
    if (!m_packet) m_packet = av_packet_alloc();
    if (!m_frame) m_frame = av_frame_alloc();
    if (!m_resampledFrame) m_resampledFrame = av_frame_alloc();

    return QIODevice::open(QIODevice::ReadOnly);
}

void AudioPlayer::close()
{
    stop();
    QMutexLocker locker(&m_mutex);
    if (m_swrCtx) {
        swr_free(&m_swrCtx);
        m_swrCtx = nullptr;
    }
    if (m_codecCtx) {
        avcodec_free_context(&m_codecCtx);
        m_codecCtx = nullptr;
    }
    if (m_formatCtx) {
        avformat_close_input(&m_formatCtx);
        m_formatCtx = nullptr;
    }
    m_buffer.clear();
    m_audioStreamIndex = -1;
    m_duration = 0;
    m_position = 0;
    m_bytesRead = 0;
    QIODevice::close();
}

qint64 AudioPlayer::readData(char *data, qint64 maxSize)
{
    QMutexLocker locker(&m_mutex);
    if (m_buffer.isEmpty()) {
        return 0;
    }

    qint64 readLen = qMin(static_cast<qint64>(m_buffer.size()), maxSize);
    memcpy(data, m_buffer.constData(), readLen);
    m_buffer.remove(0, readLen);
    m_bytesRead += readLen;
    return readLen;
}

qint64 AudioPlayer::writeData(const char *data, qint64 maxSize)
{
    Q_UNUSED(data); Q_UNUSED(maxSize);
    return 0;
}

qint64 AudioPlayer::bytesAvailable() const
{
    QMutexLocker locker(&m_mutex);
    return m_buffer.size() + QIODevice::bytesAvailable();
}

qint64 AudioPlayer::duration() const { return m_duration; }

qint64 AudioPlayer::position() const
{
    // 44100Hz * 2 channels * 2 bytes (16bit) = 176400 bytes/sec
    // ms = bytes / 176.4
    return m_bytesRead / 176.4;
}

void AudioPlayer::setPosition(qint64 pos)
{
    QMutexLocker locker(&m_mutex);
    if (!m_formatCtx) return;

    int64_t timestamp = pos * 1000;
    if (av_seek_frame(m_formatCtx, -1, timestamp, AVSEEK_FLAG_BACKWARD) >= 0) {
        if (m_codecCtx) avcodec_flush_buffers(m_codecCtx);
        m_position = pos;
        m_bytesRead = pos * 176.4;
        m_buffer.clear();
    }
}

void AudioPlayer::setVolume(qreal volume)
{
    // Volume is now handled by QAudioSink in MainWindow for better responsiveness
    Q_UNUSED(volume);
}

bool AudioPlayer::isPlaying() const { return m_playing; }

void AudioPlayer::play()
{
    if (m_playing && !m_paused) return;

    m_paused = false;
    if (!m_playing) {
        m_playing = true;
        m_stopDecoding = false;
        m_decodeThread = QThread::create([this](){ decodingLoop(); });
        m_decodeThread->start();
    }
}

void AudioPlayer::pause() { m_paused = true; }

void AudioPlayer::stop()
{
    m_stopDecoding = true;
    if (m_decodeThread) {
        if (m_decodeThread->isRunning()) {
            m_decodeThread->wait();
        }
        delete m_decodeThread;
        m_decodeThread = nullptr;
    }
    m_playing = false;
    m_paused = false;
    m_stopDecoding = false;
}

void AudioPlayer::decodingLoop()
{
    while (!m_stopDecoding) {
        if (m_paused) {
            QThread::msleep(10);
            continue;
        }

        {
            QMutexLocker locker(&m_mutex);
            if (m_buffer.size() > 512 * 1024) { // 减小缓冲区大小到 512KB
                locker.unlock();
                QThread::msleep(10);
                continue;
            }
        }

        if (!decodeFrame()) {
            QThread::msleep(10);
        }
    }
}

bool AudioPlayer::decodeFrame()
{
    QMutexLocker locker(&m_mutex);
    if (!m_formatCtx || !m_codecCtx || !m_swrCtx || !m_packet || !m_frame || !m_resampledFrame) {
        return false;
    }

    if (av_read_frame(m_formatCtx, m_packet) < 0) return false;

    bool success = false;
    if (m_packet->stream_index == m_audioStreamIndex) {
        if (avcodec_send_packet(m_codecCtx, m_packet) >= 0) {
            while (avcodec_receive_frame(m_codecCtx, m_frame) >= 0) {
                int outSamples = av_rescale_rnd(swr_get_delay(m_swrCtx, m_frame->sample_rate) + m_frame->nb_samples, 
                                               44100, m_frame->sample_rate, AV_ROUND_UP);
                
                av_frame_unref(m_resampledFrame);
                m_resampledFrame->nb_samples = outSamples;
                m_resampledFrame->format = AV_SAMPLE_FMT_S16;
                m_resampledFrame->sample_rate = 44100;
                av_channel_layout_default(&m_resampledFrame->ch_layout, 2);
                
                if (av_frame_get_buffer(m_resampledFrame, 0) >= 0) {
                    int converted = swr_convert(m_swrCtx, m_resampledFrame->data, outSamples, 
                                             (const uint8_t **)m_frame->data, m_frame->nb_samples);
                    if (converted > 0) {
                        int outSize = converted * 2 * 2; // 2 channels, 16-bit (2 bytes)
                        QByteArray audioData((char *)m_resampledFrame->data[0], outSize);
                        m_buffer.append(audioData);
                        if (m_frame->pts != AV_NOPTS_VALUE) {
                            m_position = av_rescale_q(m_frame->pts, m_formatCtx->streams[m_audioStreamIndex]->time_base, {1, 1000});
                        }
                        success = true;
                    }
                }
                av_frame_unref(m_frame);
            }
        }
    }
    av_packet_unref(m_packet);
    return success;
}

void AudioPlayer::setupResampler()
{
    if (m_swrCtx) swr_free(&m_swrCtx);
    AVChannelLayout out_ch_layout;
    av_channel_layout_default(&out_ch_layout, 2);
    int ret = swr_alloc_set_opts2(&m_swrCtx, &out_ch_layout, AV_SAMPLE_FMT_S16, 44100,
                        &m_codecCtx->ch_layout, m_codecCtx->sample_fmt, m_codecCtx->sample_rate, 0, nullptr);
    if (ret < 0 || swr_init(m_swrCtx) < 0) {
        if (m_swrCtx) swr_free(&m_swrCtx);
        m_swrCtx = nullptr;
    }
}

void AudioPlayer::extractMetadata(const QString &filePath, QString &title, QString &artist, QString &album, qint64 &duration)
{
    AVFormatContext *fmtCtx = nullptr;
    if (avformat_open_input(&fmtCtx, filePath.toUtf8().constData(), nullptr, nullptr) != 0) return;
    if (avformat_find_stream_info(fmtCtx, nullptr) >= 0) {
        duration = fmtCtx->duration / 1000;
        AVDictionaryEntry *tag = nullptr;
        tag = av_dict_get(fmtCtx->metadata, "title", nullptr, 0);
        if (tag) title = QString::fromUtf8(tag->value);
        tag = av_dict_get(fmtCtx->metadata, "artist", nullptr, 0);
        if (tag) artist = QString::fromUtf8(tag->value);
        tag = av_dict_get(fmtCtx->metadata, "album", nullptr, 0);
        if (tag) album = QString::fromUtf8(tag->value);
    }
    avformat_close_input(&fmtCtx);
}
