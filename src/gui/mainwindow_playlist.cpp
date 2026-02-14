#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QMenu>
#include <QTabBar>
#include <QRandomGenerator>
#include <QHBoxLayout>
#include <QToolButton>

void MainWindow::onAddFiles()
{
    QString filter = tr("音频文件") + " ("
                     "*.mp3 *.wav *.flac *.ape *.alac *.wavpack *.m4a *.aac *.ogg *.opus *.wma "
                     "*.ac3 *.dts *.mp2 *.mp1 *.mka *.mpc *.tps *.wv *.tta *.amr *.ra *.m4b "
                     "*.m4p *.pcm *.aif *.aiff *.aifc *.caf *.adts *.asf *.rm *.au *.snd "
                     "*.3gp *.3g2 *.vqv *.vql *.vqy"
                     ");;" +
                     tr("所有文件") + " (*.*)";
    QStringList filePaths = QFileDialog::getOpenFileNames(this, tr("添加音频文件"), QDir::homePath(), filter);
    if (filePaths.isEmpty()) return;
    
    int addedCount = 0;
    int dupeCount = 0;
    QList<PlaylistItem> &items = currentPlaylistItems();
    for (const QString &path : filePaths) {
        bool duplicate = false;
        for (const auto &it : items) { if (it.path == path) { duplicate = true; break; } }
        if (!duplicate) {
            items.append(PlaylistItem(path));
            addedCount++;
        } else {
            dupeCount++;
        }
    }
    
    updatePlaylistDisplay();
    saveAllPlaylists();
    if (currentMode == Shuffle) regenerateShuffleList();
    if (currentIndex == -1 && !items.isEmpty()) {
        currentIndex = 0;
        ui->playlistWidget->setCurrentRow(0);
        playAt(0);
    }
    
    QString msg = tr("成功添加 %1 首歌曲").arg(addedCount);
    if (dupeCount > 0) msg += tr("，重复 %1 首歌曲").arg(dupeCount);
    QMessageBox::information(this, tr("完成"), msg);
}

void MainWindow::onAddFolder()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("选择音乐文件夹"), QDir::homePath(), QFileDialog::ShowDirsOnly);
    if (folderPath.isEmpty()) return;
    QDir dir(folderPath);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::Readable);
    QFileInfoList fileList = dir.entryInfoList();
    if (fileList.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("该文件夹中没有找到音频文件！"));
        return;
    }
    
    int addedCount = 0;
    int dupeCount = 0;
    QList<PlaylistItem> &items = currentPlaylistItems();
    for (const QFileInfo &fileInfo : fileList) {
        QString path = fileInfo.absoluteFilePath();
        bool duplicate = false;
        for (const auto &it : items) { if (it.path == path) { duplicate = true; break; } }
        if (!duplicate) {
            items.append(PlaylistItem(path));
            addedCount++;
        } else {
            dupeCount++;
        }
    }
    
    updatePlaylistDisplay();
    saveAllPlaylists();
    if (currentMode == Shuffle) regenerateShuffleList();
    if (currentIndex == -1 && !items.isEmpty()) {
        currentIndex = 0;
        ui->playlistWidget->setCurrentRow(0);
        playAt(0);
    }
    
    QString msg = tr("成功添加 %1 首歌曲").arg(addedCount);
    if (dupeCount > 0) msg += tr("，重复 %1 首歌曲").arg(dupeCount);
    QMessageBox::information(this, tr("完成"), msg);
}

void MainWindow::onNewPlaylist() {
    bool ok;
    QString text = QInputDialog::getText(this, tr("新建列表"), tr("名称:"), QLineEdit::Normal, tr("新列表"), &ok);
    if (ok && !text.isEmpty() && !m_playlistMap.contains(text)) {
        m_playlistMap[text] = QList<PlaylistItem>();
        playlistTabBar->setCurrentIndex(playlistTabBar->addTab(text));
        saveAllPlaylists();
    }
}

void MainWindow::onTabChanged(int index) {
    if (index < 0) return;
    QString name = playlistTabBar->tabText(index);
    if (name == m_currentPlaylistName) return;
    m_currentPlaylistName = name;
    currentIndex = -1;
    audioPlayer->stop();
    updatePlaylistDisplay();
}

void MainWindow::onTabMoved(int f, int t) { Q_UNUSED(f); Q_UNUSED(t); if (m_saveTimer) m_saveTimer->start(1000); }

void MainWindow::onTabDoubleClicked(int index) {
    if (index < 0) return;
    QString old = playlistTabBar->tabText(index);
    bool ok;
    QString nw = QInputDialog::getText(this, tr("重命名"), tr("新名称:"), QLineEdit::Normal, old, &ok);
    if (ok && !nw.isEmpty() && nw != old && !m_playlistMap.contains(nw)) {
        m_playlistMap[nw] = m_playlistMap.take(old);
        playlistTabBar->setTabText(index, nw);
        if (m_currentPlaylistName == old) m_currentPlaylistName = nw;
        saveAllPlaylists();
    }
}

void MainWindow::onTabContextMenu(const QPoint &pos) {
    int idx = playlistTabBar->tabAt(pos);
    if (idx < 0) return;
    QMenu m;
    m.addAction(tr("删除列表"), [this, idx]() {
        if (playlistTabBar->count() <= 1) return;
        QString n = playlistTabBar->tabText(idx);
        if (QMessageBox::question(this, tr("确认"), tr("删除 %1 ?").arg(n)) == QMessageBox::Yes) {
            m_playlistMap.remove(n);
            playlistTabBar->removeTab(idx);
            saveAllPlaylists();
        }
    });
    m.exec(playlistTabBar->mapToGlobal(pos));
}

void MainWindow::onToggleCheckMode() {
    m_checkMode = btnCheck->isChecked();
    ui->playlistWidget->setSelectionMode(m_checkMode ? QAbstractItemView::MultiSelection : QAbstractItemView::SingleSelection);
    if (!m_checkMode) ui->playlistWidget->clearSelection();
    updatePlaylistDisplay();
}

void MainWindow::onSelectionChanged() {
    if (!m_checkMode) return;
    QSignalBlocker b(ui->playlistWidget);
    for (int i = 0; i < ui->playlistWidget->count(); ++i) {
        ui->playlistWidget->item(i)->setCheckState(ui->playlistWidget->item(i)->isSelected() ? Qt::Checked : Qt::Unchecked);
    }
}

void MainWindow::onSelectAll() {
    bool c = btnSelectAll->isChecked();
    if (c) {
        if (!m_checkMode) btnCheck->setChecked(true);
        for (int i = 0; i < ui->playlistWidget->count(); ++i) {
            ui->playlistWidget->item(i)->setCheckState(Qt::Checked);
            ui->playlistWidget->item(i)->setSelected(true);
        }
    } else if (m_checkMode) btnCheck->setChecked(false);
}

void MainWindow::onSortPlaylist(int index)
{
    QList<PlaylistItem> &items = currentPlaylistItems();
    if (items.isEmpty() || index == 0) return;
    QString curPath = (currentIndex >= 0 && currentIndex < items.size()) ? items[currentIndex].path : "";
    
    switch (index) {
    case 1: std::sort(items.begin(), items.end(), [](const PlaylistItem &a, const PlaylistItem &b) { return a.addedTime < b.addedTime; }); break;
    case 2: std::sort(items.begin(), items.end(), [](const PlaylistItem &a, const PlaylistItem &b) { return a.title.toLower() < b.title.toLower(); }); break;
    case 3: std::sort(items.begin(), items.end(), [](const PlaylistItem &a, const PlaylistItem &b) { return a.artist.toLower() < b.artist.toLower(); }); break;
    case 4: std::sort(items.begin(), items.end(), [](const PlaylistItem &a, const PlaylistItem &b) { return a.album.toLower() < b.album.toLower(); }); break;
    case 5: std::sort(items.begin(), items.end(), [](const PlaylistItem &a, const PlaylistItem &b) { return a.duration < b.duration; }); break;
    case 6: std::sort(items.begin(), items.end(), [](const PlaylistItem &a, const PlaylistItem &b) { return a.playCount > b.playCount; }); break;
    case 7: for (int i = items.size() - 1; i > 0; --i) items.swapItemsAt(i, QRandomGenerator::global()->bounded(i + 1)); break;
    }
    
    updatePlaylistDisplay();
    if (!curPath.isEmpty()) {
        for (int i = 0; i < items.size(); ++i) { if (items[i].path == curPath) { currentIndex = i; ui->playlistWidget->setCurrentRow(i); break; } }
    }
    if (currentMode == Shuffle) regenerateShuffleList();
    saveAllPlaylists();
}

void MainWindow::onDeleteSelected()
{
    QList<PlaylistItem> &items = currentPlaylistItems();
    QList<QListWidgetItem*> sel = ui->playlistWidget->selectedItems();
    if (sel.isEmpty()) return;
    QString curPath = (currentIndex >= 0 && currentIndex < items.size()) ? items[currentIndex].path : "";
    bool curDel = false;
    QList<int> rows;
    for (auto i : sel) rows.append(ui->playlistWidget->row(i));
    std::sort(rows.begin(), rows.end(), std::greater<int>());
    for (int r : rows) { if (r == currentIndex) curDel = true; items.removeAt(r); }
    updatePlaylistDisplay();
    if (!curPath.isEmpty()) { for (int i = 0; i < items.size(); ++i) { if (items[i].path == curPath) { currentIndex = i; break; } } }
    if (curDel) { audioPlayer->stop(); currentIndex = -1; ui->currentSongLabel->setText(tr("未播放")); }
    else if (currentIndex != -1) ui->playlistWidget->setCurrentRow(currentIndex);
    saveAllPlaylists();
}

void MainWindow::setupPlaylistUI()
{
    ui->playlistWidget->setItemDelegate(new PlaylistDelegate(ui->playlistWidget));
    ui->playlistWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->playlistWidget->setMouseTracking(true);
    
    ui->playlistWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->playlistWidget, &QListWidget::customContextMenuRequested, this, &MainWindow::onSongContextMenu);

    QHBoxLayout *plLayout = new QHBoxLayout();
    playlistTabBar = new QTabBar(this);
    playlistTabBar->setMovable(true);
    playlistTabBar->setShape(QTabBar::RoundedNorth);
    playlistTabBar->setContextMenuPolicy(Qt::CustomContextMenu);
    playlistTabBar->setElideMode(Qt::ElideRight);
    playlistTabBar->setExpanding(false);
    playlistTabBar->setStyleSheet(
        "QTabBar::tab {"
        "  background: #f0f0f0;"
        "  border: 1px solid #c4c4c4;"
        "  padding: 6px 15px;"
        "  margin-right: 2px;"
        "  border-top-left-radius: 4px;"
        "  border-top-right-radius: 4px;"
        "  min-width: 80px;"
        "}"
        "QTabBar::tab:selected {"
        "  background: #ffffff;"
        "  border-bottom-color: #ffffff;"
        "  font-weight: bold;"
        "}"
        "QTabBar::tab:hover:!selected {"
        "  background: #e8e8e8;"
        "}"
    );

    QToolButton *btnNew = new QToolButton(this);
    btnNew->setText(tr("新建列表"));
    btnCheck = new QToolButton(this);
    btnCheck->setText(tr("多选"));
    btnCheck->setCheckable(true);
    btnSelectAll = new QToolButton(this);
    btnSelectAll->setText(tr("全选"));
    btnSelectAll->setCheckable(true);

    plLayout->addWidget(playlistTabBar, 1);
    plLayout->addWidget(btnNew);
    plLayout->addWidget(btnCheck);
    plLayout->addWidget(btnSelectAll);
    ui->verticalLayout->insertLayout(1, plLayout);

    ui->selectAllButton->hide(); // Hide old button

    connect(btnNew, &QToolButton::clicked, this, &MainWindow::onNewPlaylist);
    connect(btnCheck, &QToolButton::toggled, this, &MainWindow::onToggleCheckMode);
    connect(btnSelectAll, &QToolButton::clicked, this, &MainWindow::onSelectAll);
    connect(ui->playlistWidget, &QListWidget::itemSelectionChanged, this, &MainWindow::onSelectionChanged);
    connect(ui->playlistWidget, &QListWidget::itemChanged, this, [this](QListWidgetItem *item){
        if (m_checkMode) {
            QSignalBlocker blocker(ui->playlistWidget);
            item->setSelected(item->checkState() == Qt::Checked);
        }
    });

    connect(playlistTabBar, &QTabBar::currentChanged, this, &MainWindow::onTabChanged);
    connect(playlistTabBar, &QTabBar::tabMoved, this, &MainWindow::onTabMoved);
    connect(playlistTabBar, &QTabBar::tabBarDoubleClicked, this, &MainWindow::onTabDoubleClicked);
    connect(playlistTabBar, &QTabBar::customContextMenuRequested, this, &MainWindow::onTabContextMenu);
}

void MainWindow::onSongContextMenu(const QPoint &pos) {
    auto item = ui->playlistWidget->itemAt(pos);
    if (!item && !m_checkMode) return;
    QMenu m;
    auto addM = m.addMenu(tr("添加到"));
    for (const auto &n : m_playlistMap.keys()) {
        if (n == m_currentPlaylistName) continue;
        addM->addAction(n, [this, n]() {
            auto &d = m_playlistMap[n];
            auto &s = currentPlaylistItems();
            QList<int> rows;
            for (int i = 0; i < ui->playlistWidget->count(); ++i) {
                if (m_checkMode ? ui->playlistWidget->item(i)->checkState() == Qt::Checked : ui->playlistWidget->item(i)->isSelected()) rows.append(i);
            }
            for (int r : rows) {
                const auto &p = s[r];
                bool ex = false;
                for (const auto &dp : d) if (dp.path == p.path) { ex = true; break; }
                if (!ex) d.append(p);
            }
            saveAllPlaylists();
        });
    }
    m.exec(ui->playlistWidget->mapToGlobal(pos));
}
