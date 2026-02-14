#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileInfo>

void MainWindow::onPlayPause()
{
    if (currentIndex == -1) {
        if (currentPlaylistItems().isEmpty()) {
            return;
        }
        currentIndex = 0;
        ui->playlistWidget->setCurrentRow(0);
        playAt(0);
        return;
    }
    if (audioPlayer && audioPlayer->isPlaying()) {
        audioPlayer->pause();
        audioSink->suspend();
        ui->playPauseButton->setText(tr("播放"));
    } else {
        audioPlayer->play();
        if (audioSink->state() == QAudio::SuspendedState) audioSink->resume();
        else if (audioSink->state() == QAudio::StoppedState) audioSink->start(audioPlayer);
        ui->playPauseButton->setText(tr("暂停"));
    }
}

void MainWindow::onNext()
{
    QList<PlaylistItem> &items = currentPlaylistItems();
    if (items.isEmpty()) return;
    if (currentMode == Shuffle) {
        if (shuffledIndices.isEmpty()) regenerateShuffleList();
        int idx = shuffledIndices.indexOf(currentIndex);
        idx = (idx + 1) % shuffledIndices.size();
        currentIndex = shuffledIndices[idx];
    } else {
        currentIndex = (currentIndex + 1) % items.size();
    }
    ui->playlistWidget->setCurrentRow(currentIndex);
    playAt(currentIndex);
}

void MainWindow::onPrev()
{
    QList<PlaylistItem> &items = currentPlaylistItems();
    if (items.isEmpty()) return;
    if (currentMode == Shuffle) {
        if (shuffledIndices.isEmpty()) regenerateShuffleList();
        int idx = shuffledIndices.indexOf(currentIndex);
        idx = (idx - 1 + shuffledIndices.size()) % shuffledIndices.size();
        currentIndex = shuffledIndices[idx];
    } else {
        currentIndex = (currentIndex - 1 + items.size()) % items.size();
    }
    ui->playlistWidget->setCurrentRow(currentIndex);
    playAt(currentIndex);
}

void MainWindow::playAt(int index) {
    QList<PlaylistItem> &items = currentPlaylistItems();
    if (index < 0 || index >= items.size()) return;
    items[index].playCount++;
    QString p = items[index].path;
    audioPlayer->stop();
    if (audioPlayer->openFile(p)) {
        audioSink->stop();
        audioPlayer->play();
        audioSink->start(audioPlayer);
        ui->playPauseButton->setText(tr("暂停"));
        ui->currentSongLabel->setText(QFileInfo(p).fileName());
        setWindowTitle("V");
        saveAllPlaylists();
    }
}

void MainWindow::onPositionUpdate()
{
    if (audioPlayer) {
        qint64 pos = audioPlayer->position();
        qint64 dur = audioPlayer->duration();
        if (!ui->progressSlider->isSliderDown()) ui->progressSlider->setValue(static_cast<int>(pos));
        ui->currentTimeLabel->setText(formatTime(pos));
        if (dur > 0) {
            ui->progressSlider->setRange(0, static_cast<int>(dur));
            ui->totalTimeLabel->setText(formatTime(dur));
        }
    }
}

void MainWindow::onSliderMoved(int pos) { if (audioPlayer) audioPlayer->setPosition(pos); }
void MainWindow::onVolumeChanged(int vol) { if (audioSink) audioSink->setVolume(vol / 100.0); }

void MainWindow::onModeChanged(int index)
{
    currentMode = static_cast<PlayMode>(ui->modeComboBox->itemData(index).toInt());
    if (currentMode == Shuffle) regenerateShuffleList();
}

void MainWindow::onPlaylistItemDoubleClicked(QListWidgetItem *item) { currentIndex = ui->playlistWidget->row(item); playAt(currentIndex); }

