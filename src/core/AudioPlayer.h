#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QIODevice>
#include <QByteArray>
#include <QMutex>
#include <QThread>
#include <QTimer>
#include <atomic>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
#include <libavutil/channel_layout.h>
}

class AudioPlayer : public QIODevice
{
    Q_OBJECT

public:
    explicit AudioPlayer(QObject *parent = nullptr);
    ~AudioPlayer();

    bool openFile(const QString &filePath);
    void close() override;

    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;
    qint64 bytesAvailable() const override;
    bool isSequential() const override { return true; }

    qint64 duration() const;
    qint64 position() const;
    void setPosition(qint64 pos);

    void setVolume(qreal volume);

    bool isPlaying() const;
    void play();
    void pause();
    void stop();

    static void extractMetadata(const QString &filePath, QString &title, QString &artist, QString &album, qint64 &duration);

private:
    void decodingLoop(); // 后台解码循环
    bool decodeFrame();
    void setupResampler();

    AVFormatContext *m_formatCtx = nullptr;
    AVCodecContext *m_codecCtx = nullptr;
    SwrContext *m_swrCtx = nullptr;
    AVPacket *m_packet = nullptr;
    AVFrame *m_frame = nullptr;
    AVFrame *m_resampledFrame = nullptr;
    int m_audioStreamIndex = -1;

    QByteArray m_buffer; // 生产者-消费者缓冲区
    mutable QMutex m_mutex;
    
    qint64 m_duration = 0;
    qint64 m_position = 0;
    std::atomic<qint64> m_bytesRead{0}; // Track playback position safely
    bool m_playing = false;
    bool m_paused = false;
    bool m_stopDecoding = false;
    qreal m_volume = 1.0;

    QThread *m_decodeThread = nullptr;
};

#endif // AUDIOPLAYER_H
