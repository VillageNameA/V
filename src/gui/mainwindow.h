#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QAudioSink>
#include <QTimer>
#include <QDateTime>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QSystemTrayIcon>
#include <QMap>
#include "core/AudioPlayer.h"
#include "PlaylistDelegate.h"
#include "PlaylistItem.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QLocalServer; 
class QTabBar;
class QListWidgetItem;
class QToolButton;
class QKeyEvent;
class QCloseEvent;
class QComboBox;
class QAction;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum PlayMode {
        Sequential,    // 顺序播放
        SingleLoop,    // 单曲循环
        ListLoop,      // 列表循环
        Shuffle        // 随机播放
    };

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onAddFiles();
    void onAddFolder();
    void onPlayPause();
    void onNext();
    void onPrev();
    void onSliderMoved(int position);
    void onVolumeChanged(int volume);
    void onModeChanged(int index);
    void onSortPlaylist(int index);
    void onSelectAll();
    void onSelectionChanged();
    void onToggleCheckMode();
    void onDeleteSelected();
    void onPlaylistItemDoubleClicked(QListWidgetItem *item);
    void onPositionUpdate();
    
    void onNewPlaylist();
    void onTabChanged(int index);
    void onTabMoved(int from, int to);
    void onTabDoubleClicked(int index);
    void onTabContextMenu(const QPoint &pos);
    void onSongContextMenu(const QPoint &pos);
    
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onMessageReceived(); 
    void onLanguageChanged(int index);

protected:
    void changeEvent(QEvent *event) override;

private:
    void retranslateUi();
    QComboBox *languageComboBox = nullptr;
    QAction *m_restoreAction = nullptr;
    QAction *m_quitAction = nullptr;
    bool m_languageInitialized = false;
    bool m_forceClose = false;
    
    Ui::MainWindow *ui = nullptr;
    QTabBar *playlistTabBar = nullptr;
    QToolButton *btnCheck = nullptr;
    QToolButton *btnSelectAll = nullptr;
    QAudioSink *audioSink = nullptr;
    AudioPlayer *audioPlayer = nullptr;
    QSystemTrayIcon *trayIcon = nullptr;
    QLocalServer *localServer = nullptr; 
    
    QMap<QString, QList<PlaylistItem>> m_playlistMap;
    QString m_currentPlaylistName;
    bool m_checkMode = false;
    
    QTimer *m_saveTimer = nullptr;
    
    QList<PlaylistItem> &currentPlaylistItems();
    QList<int> shuffledIndices;
    int currentIndex = -1;
    PlayMode currentMode = Sequential;
    
    void playAt(int index);
    void saveAllPlaylists();
    void loadAllPlaylists();
    void updatePlaylistDisplay();
    void regenerateShuffleList();
    QString formatTime(qint64 ms);
    void setupPlaylistUI();
    void setupTrayIcon();
};

#endif // MAINWINDOW_H
