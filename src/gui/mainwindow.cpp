#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSettings>
#include <QCheckBox>
#include <QPushButton>
#include <QMessageBox>
#include <QApplication>
#include <QLocalServer>
#include <QPainter>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QMenu>
#include <QClipboard>
#include <QRandomGenerator>
#include <QAudioFormat>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QSlider>
#include <QComboBox>
#include <QAction>
#include <QProcess>
#include <QSignalBlocker>
#include <QDateTime>
#include <QFile>
#include <QTextStream>

static void appendLanguageLog(const QString &message)
{
    QFile logFile(QDir::tempPath() + "/V_language_startup.log");
    if (!logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return;
    }
    QTextStream out(&logFile);
    out << QDateTime::currentDateTime().toString(Qt::ISODate) << " " << message << "\n";
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    // 从资源加载程序图标
    setWindowIcon(QIcon(":/app_icon.ico"));
    
    // 初始化单实例通信服务器
    localServer = new QLocalServer(this);
    localServer->removeServer("V_MusicPlayer_Server"); 
    if (!localServer->listen("V_MusicPlayer_Server")) {
        qDebug() << "Server could not start:" << localServer->errorString();
    }
    connect(localServer, &QLocalServer::newConnection, this, &MainWindow::onMessageReceived);
    
    setupPlaylistUI();
    setupTrayIcon();

    // 在右上方添加语言切换器
    languageComboBox = new QComboBox(this);
    languageComboBox->addItem(tr("跟随系统"), "default");
    
    // 按使用人数估算的全球前50大语言
    languageComboBox->addItem("English", "en_US");
    languageComboBox->addItem("简体中文", "zh_CN");
    languageComboBox->addItem("繁體中文 (粤语/香港)", "zh_HK");
    languageComboBox->addItem("繁體中文 (台湾)", "zh_TW");
    languageComboBox->addItem("हिन्दी", "hi_IN");
    languageComboBox->addItem("Español", "es_ES");
    languageComboBox->addItem("Français", "fr_FR");
    languageComboBox->addItem("العربية", "ar_SA");
    languageComboBox->addItem("বাংলা", "bn_BD");
    languageComboBox->addItem("Português", "pt_PT");
    languageComboBox->addItem("Русский", "ru_RU");
    languageComboBox->addItem("اردو", "ur_PK");
    languageComboBox->addItem("Bahasa Indonesia", "id_ID");
    languageComboBox->addItem("Deutsch", "de_DE");
    languageComboBox->addItem("日本語", "ja_JP");
    languageComboBox->addItem("मराठी", "mr_IN");
    languageComboBox->addItem("తెలుగు", "te_IN");
    languageComboBox->addItem("Türkçe", "tr_TR");
    languageComboBox->addItem("தமிழ்", "ta_IN");
    languageComboBox->addItem("Tiếng Việt", "vi_VN");
    languageComboBox->addItem("Tagalog", "tl_PH");
    languageComboBox->addItem("한국어", "ko_KR");
    languageComboBox->addItem("فارسی", "fa_IR");
    languageComboBox->addItem("Hausa", "ha_NG");
    languageComboBox->addItem("Kiswahili", "sw_KE");
    languageComboBox->addItem("Basa Jawa", "jv_ID");
    languageComboBox->addItem("Italiano", "it_IT");
    languageComboBox->addItem("ਪੰਜਾਬੀ", "pa_PK");
    languageComboBox->addItem("ಕನ್ನಡ", "kn_IN");
    languageComboBox->addItem("ગુજરાતી", "gu_IN");
    languageComboBox->addItem("ไทย", "th_TH");
    languageComboBox->addItem("Amharic", "am_ET");
    languageComboBox->addItem("Yorùbá", "yo_NG");
    languageComboBox->addItem("မြန်မာဘာသာ", "my_MM");
    languageComboBox->addItem("پښتو", "ps_AF");
    languageComboBox->addItem("Українська", "uk_UA");
    languageComboBox->addItem("Basa Sunda", "su_ID");
    languageComboBox->addItem("Polski", "pl_PL");
    languageComboBox->addItem("O'zbek", "uz_UZ");
    languageComboBox->addItem("മലയാളം", "ml_IN");
    languageComboBox->addItem("سنڌي", "sd_PK");
    languageComboBox->addItem("Română", "ro_RO");
    languageComboBox->addItem("Azərbaycanca", "az_AZ");
    languageComboBox->addItem("Kurdî", "ku_TR");
    languageComboBox->addItem("Nederlands", "nl_NL");
    languageComboBox->addItem("Igbo", "ig_NG");
    languageComboBox->addItem("isiZulu", "zu_ZA");
    languageComboBox->addItem("Čeština", "cs_CZ");
    languageComboBox->addItem("Ελληνικά", "el_GR");
    languageComboBox->addItem("Magyar", "hu_HU");
    languageComboBox->addItem("Svenska", "sv_SE");
    
    // 寻找 addButtonsLayout 并在末尾添加
    ui->addButtonsLayout->addStretch();
    ui->addButtonsLayout->addWidget(languageComboBox);
    
    connect(languageComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onLanguageChanged);

    audioPlayer = new AudioPlayer(this);
    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(2);
    format.setSampleFormat(QAudioFormat::Int16);
    audioSink = new QAudioSink(format, this);
    audioSink->setVolume(0.5);

    ui->volumeSlider->setValue(50);

    ui->modeComboBox->addItem(tr("顺序播放"), Sequential);
    ui->modeComboBox->addItem(tr("单曲循环"), SingleLoop);
    ui->modeComboBox->addItem(tr("列表循环"), ListLoop);
    ui->modeComboBox->addItem(tr("随机播放"), Shuffle);
    currentMode = Sequential;
    currentIndex = -1;

    connect(ui->openButton, &QPushButton::clicked, this, &MainWindow::onAddFiles);
    connect(ui->addFolderButton, &QPushButton::clicked, this, &MainWindow::onAddFolder);
    connect(ui->playPauseButton, &QPushButton::clicked, this, &MainWindow::onPlayPause);
    connect(ui->prevButton, &QPushButton::clicked, this, &MainWindow::onPrev);
    connect(ui->nextButton, &QPushButton::clicked, this, &MainWindow::onNext);
    connect(ui->modeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onModeChanged);
    connect(ui->playlistWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::onPlaylistItemDoubleClicked);

    connect(ui->sortComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onSortPlaylist);
    ui->sortComboBox->clear();
    ui->sortComboBox->addItem(tr("排序方式"));
    ui->sortComboBox->addItem(tr("添加时间"));
    ui->sortComboBox->addItem(tr("歌曲名"));
    ui->sortComboBox->addItem(tr("歌手"));
    ui->sortComboBox->addItem(tr("专辑"));
    ui->sortComboBox->addItem(tr("时长"));
    ui->sortComboBox->addItem(tr("播放次数"));
    ui->sortComboBox->addItem(tr("随机排序"));
    
    connect(ui->deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteSelected);
    connect(ui->progressSlider, &QSlider::sliderMoved, this, &MainWindow::onSliderMoved);
    connect(ui->volumeSlider, &QSlider::valueChanged, this, &MainWindow::onVolumeChanged);

    loadAllPlaylists();

    m_saveTimer = new QTimer(this);
    m_saveTimer->setSingleShot(true);
    connect(m_saveTimer, &QTimer::timeout, this, &MainWindow::saveAllPlaylists);

    QTimer *positionTimer = new QTimer(this);
    connect(positionTimer, &QTimer::timeout, this, &MainWindow::onPositionUpdate);
    positionTimer->start(100);

    setWindowTitle("V");
    ui->currentSongLabel->setText(tr("未播放"));

    connect(positionTimer, &QTimer::timeout, this, [this]() {
        if (audioPlayer && audioPlayer->isPlaying() && 
            audioPlayer->position() >= audioPlayer->duration()) {
            switch (currentMode) {
            case SingleLoop:
                audioPlayer->setPosition(0);
                audioPlayer->play();
                break;
            case Sequential:
                if (currentIndex < currentPlaylistItems().size() - 1) onNext();
                else {
                    ui->playPauseButton->setText(tr("播放"));
                    ui->progressSlider->setValue(0);
                    ui->currentTimeLabel->setText("00:00");
                    ui->currentSongLabel->setText(tr("未播放"));
                    setWindowTitle("V");
                }
                break;
            case ListLoop:
            case Shuffle:
                onNext();
                break;
            }
        }
    });
}

MainWindow::~MainWindow() {
    if (m_saveTimer && m_saveTimer->isActive()) {
        m_saveTimer->stop();
        saveAllPlaylists();
    }
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_C) {
        QList<QListWidgetItem*> selected = ui->playlistWidget->selectedItems();
        if (!selected.isEmpty()) {
            QStringList texts;
            QList<PlaylistItem> &items = currentPlaylistItems();
            for (QListWidgetItem *item : selected) {
                int row = ui->playlistWidget->row(item);
                if (row >= 0 && row < items.size()) {
                    texts << QString("%1 - %2").arg(items[row].artist).arg(items[row].title);
                }
            }
            QApplication::clipboard()->setText(texts.join("\n"));
        }
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::setupTrayIcon() {
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/app_icon.ico"));
    trayIcon->setToolTip("V");

    QMenu *trayMenu = new QMenu(this);
    m_restoreAction = trayMenu->addAction(tr("还原窗口"));
    trayMenu->addSeparator();
    m_quitAction = trayMenu->addAction(tr("退出程序"));

    connect(m_restoreAction, &QAction::triggered, this, &MainWindow::showNormal);
    connect(m_quitAction, &QAction::triggered, this, [this]() {
        m_forceClose = true;
        qApp->quit();
    });

    trayIcon->setContextMenu(trayMenu);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayIconActivated);
    trayIcon->show();
}

void MainWindow::onMessageReceived() {
    // 当收到新实例的消息时，唤醒并显示当前窗口
    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    showNormal();
    raise();
    activateWindow();
}

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
        if (isHidden()) {
            showNormal();
            activateWindow();
        } else {
            hide();
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (m_forceClose) {
        event->accept();
        return;
    }

    QSettings settings("MyCompany", "MusicPlayer");
    bool neverAsk = settings.value("General/NeverAskOnClose", false).toBool();
    QString action = settings.value("General/ActionOnClose", "Minimize").toString();

    if (neverAsk) {
        if (action == "Minimize") {
            hide();
            event->ignore();
        } else {
            event->accept();
        }
        return;
    }

    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("退出确认"));
    msgBox.setText(tr("您点击了关闭按钮，请选择您的操作："));
    msgBox.setIcon(QMessageBox::Question);

    QPushButton *btnMinimize = msgBox.addButton(tr("最小化到托盘"), QMessageBox::ActionRole);
    msgBox.addButton(tr("退出程序"), QMessageBox::DestructiveRole);
    QPushButton *btnCancel = msgBox.addButton(tr("取消"), QMessageBox::RejectRole);
    msgBox.setDefaultButton(btnMinimize);

    QCheckBox *cbNeverAsk = new QCheckBox(tr("不再通知，记住我的选择"), &msgBox);
    msgBox.setCheckBox(cbNeverAsk);

    msgBox.exec();

    QAbstractButton *clickedButton = msgBox.clickedButton();
    if (clickedButton == btnCancel || clickedButton == nullptr) {
        event->ignore();
        return;
    }

    if (cbNeverAsk->isChecked()) {
        settings.setValue("General/NeverAskOnClose", true);
        settings.setValue("General/ActionOnClose", (clickedButton == btnMinimize) ? "Minimize" : "Quit");
    }

    if (clickedButton == btnMinimize) {
        hide();
        event->ignore();
    } else {
        event->accept();
    }
}

QList<PlaylistItem> &MainWindow::currentPlaylistItems()
{
    if (!m_playlistMap.contains(m_currentPlaylistName)) {
        m_playlistMap[m_currentPlaylistName] = QList<PlaylistItem>();
    }
    return m_playlistMap[m_currentPlaylistName];
}

void MainWindow::updatePlaylistDisplay()
{
    ui->playlistWidget->clear();
    QList<PlaylistItem> &items = currentPlaylistItems();
    int total = items.size();
    int padWidth = qMax(2, (int)QString::number(total).length());

    for (int i = 0; i < items.size(); ++i) {
        const PlaylistItem &item = items[i];
        QString serialNum = QString("%1").arg(i + 1, padWidth, 10, QChar('0'));
        QString displayText = QString("%1    %2 - %3 [%4]")
                                  .arg(serialNum)
                                  .arg(item.artist)
                                  .arg(item.title)
                                  .arg(formatTime(item.duration));
        
        QListWidgetItem *listItem = new QListWidgetItem(displayText);
        listItem->setForeground(QColor("#333333"));
        if (i == currentIndex) {
            QFont font = listItem->font();
            font.setBold(true);
            listItem->setFont(font);
            listItem->setForeground(QColor("#0078d7"));
            listItem->setBackground(QColor("#e5f1fb"));
        }
        if (m_checkMode) {
            listItem->setFlags(listItem->flags() | Qt::ItemIsUserCheckable);
            listItem->setCheckState(Qt::Unchecked);
        } else {
            listItem->setFlags(listItem->flags() & ~Qt::ItemIsUserCheckable);
        }
        ui->playlistWidget->addItem(listItem);
    }
}

void MainWindow::saveAllPlaylists() {
    QSettings s("MyCompany", "MusicPlayer");
    s.beginGroup("Playlists");
    s.remove("");
    QStringList ord;
    for (int i = 0; i < playlistTabBar->count(); ++i) ord.append(playlistTabBar->tabText(i));
    s.setValue("Names", ord);
    s.setValue("Current", m_currentPlaylistName);
    for (const auto &n : ord) {
        s.beginWriteArray(n);
        const auto &its = m_playlistMap[n];
        for (int i = 0; i < its.size(); ++i) {
            s.setArrayIndex(i);
            s.setValue("path", its[i].path);
            s.setValue("title", its[i].title);
            s.setValue("artist", its[i].artist);
            s.setValue("album", its[i].album);
            s.setValue("duration", its[i].duration);
            s.setValue("playCount", its[i].playCount);
            s.setValue("addedTime", its[i].addedTime);
        }
        s.endArray();
    }
    s.endGroup();
    s.setValue("mode", static_cast<int>(currentMode));
}

void MainWindow::loadAllPlaylists() {
    QSettings s("MyCompany", "MusicPlayer");
    s.beginGroup("Playlists");
    auto names = s.value("Names").toStringList();
    if (names.isEmpty()) names << tr("默认列表");
    m_playlistMap.clear();
    for (const auto &n : names) {
        int sz = s.beginReadArray(n);
        QList<PlaylistItem> its;
        for (int i = 0; i < sz; ++i) {
            s.setArrayIndex(i);
            PlaylistItem it;
            it.path = s.value("path").toString();
            it.title = s.value("title").toString();
            it.artist = s.value("artist").toString();
            it.album = s.value("album").toString();
            it.duration = s.value("duration").toLongLong();
            it.playCount = s.value("playCount").toInt();
            it.addedTime = s.value("addedTime").toDateTime();
            its.append(it);
        }
        s.endArray();
        m_playlistMap[n] = its;
    }
    s.endGroup();
    playlistTabBar->blockSignals(true);
    while (playlistTabBar->count() > 0) playlistTabBar->removeTab(0);
    for (const auto &n : names) playlistTabBar->addTab(n);
    m_currentPlaylistName = s.value("Playlists/Current", "默认列表").toString();
    int idx = 0;
    for (int i = 0; i < playlistTabBar->count(); ++i) if (playlistTabBar->tabText(i) == m_currentPlaylistName) { idx = i; break; }
    playlistTabBar->setCurrentIndex(idx);
    playlistTabBar->blockSignals(false);
    currentMode = static_cast<PlayMode>(s.value("mode", Sequential).toInt());
    ui->modeComboBox->setCurrentIndex(ui->modeComboBox->findData(currentMode));

    // 加载语言设置
    QString lang = s.value("General/Language", "default").toString();
    int langIdx = languageComboBox->findData(lang);
    if (langIdx != -1) {
        const QSignalBlocker blocker(languageComboBox);
        languageComboBox->setCurrentIndex(langIdx);
        // 手动处理一次初始化逻辑（在 onLanguageChanged 中处理）
        onLanguageChanged(langIdx);
    } else {
        const int defaultIdx = languageComboBox->findData("default");
        if (defaultIdx != -1) {
            const QSignalBlocker blocker(languageComboBox);
            languageComboBox->setCurrentIndex(defaultIdx);
            onLanguageChanged(defaultIdx);
        }
        s.setValue("General/Language", "default");
        s.sync();
    }

    updatePlaylistDisplay();
}


void MainWindow::regenerateShuffleList() {
    shuffledIndices.clear();
    auto &its = currentPlaylistItems();
    for (int i = 0; i < its.size(); ++i) shuffledIndices.append(i);
    for (int i = shuffledIndices.size() - 1; i > 0; --i) shuffledIndices.swapItemsAt(i, QRandomGenerator::global()->bounded(i + 1));
    if (currentIndex != -1 && shuffledIndices.contains(currentIndex)) {
        shuffledIndices.removeAll(currentIndex);
        shuffledIndices.prepend(currentIndex);
    }
}

void MainWindow::onLanguageChanged(int index) {
    if (index < 0) return;
    QString lang = languageComboBox->itemData(index).toString();
    QSettings settings("MyCompany", "MusicPlayer");
    QString oldLang = settings.value("General/Language", "default").toString();

    if (!m_languageInitialized) {
        m_languageInitialized = true;
        return;
    }

    if (lang == oldLang) return;

    settings.setValue("General/Language", lang);
    settings.sync();
    appendLanguageLog(QString("ui-change old=%1 new=%2").arg(oldLang, lang));

    QMessageBox::StandardButton reply = QMessageBox::question(this,
        tr("语言变更"),
        tr("语言设置已修改，需要重启程序才能生效。是否立即重启？"),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QStringList args = qApp->arguments();
        args.removeAll("--restart");
        args.append("--restart");
        appendLanguageLog("ui-restart-confirmed true");
        m_forceClose = true;
        QProcess::startDetached(qApp->applicationFilePath(), args);
        qApp->quit();
    } else {
        appendLanguageLog("ui-restart-confirmed false");
    }
}

void MainWindow::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::retranslateUi() {
    ui->retranslateUi(this);
    // 更新手动创建的UI元素
    if (languageComboBox) {
        languageComboBox->setItemText(0, tr("跟随系统"));
    }
    if (m_restoreAction) {
        m_restoreAction->setText(tr("还原窗口"));
    }
    if (m_quitAction) {
        m_quitAction->setText(tr("退出程序"));
    }
    // 更新窗口标题
    setWindowTitle("V");
    
    // 更新模式列表
    int curIdx = ui->modeComboBox->currentIndex();
    ui->modeComboBox->clear();
    ui->modeComboBox->addItem(tr("顺序播放"), Sequential);
    ui->modeComboBox->addItem(tr("单曲循环"), SingleLoop);
    ui->modeComboBox->addItem(tr("列表循环"), ListLoop);
    ui->modeComboBox->addItem(tr("随机播放"), Shuffle);
    ui->modeComboBox->setCurrentIndex(curIdx);

    // 更新排序列表
    int sortIdx = ui->sortComboBox->currentIndex();
    ui->sortComboBox->clear();
    ui->sortComboBox->addItem(tr("排序方式"));
    ui->sortComboBox->addItem(tr("添加时间"));
    ui->sortComboBox->addItem(tr("歌曲名"));
    ui->sortComboBox->addItem(tr("歌手"));
    ui->sortComboBox->addItem(tr("专辑"));
    ui->sortComboBox->addItem(tr("时长"));
    ui->sortComboBox->addItem(tr("播放次数"));
    ui->sortComboBox->addItem(tr("随机排序"));
    ui->sortComboBox->setCurrentIndex(sortIdx);
}


QString MainWindow::formatTime(qint64 ms) {
    qint64 s = ms / 1000, m = s / 60;
    s %= 60;
    return QString("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
}
