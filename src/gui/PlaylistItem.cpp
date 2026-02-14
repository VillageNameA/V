#include "PlaylistItem.h"
#include <QFileInfo>
#include "core/AudioPlayer.h"

PlaylistItem::PlaylistItem(const QString &p) : path(p), addedTime(QDateTime::currentDateTime()) {
    QFileInfo info(p);
    title = info.completeBaseName();
    artist = "未知歌手";
    album = "未知专辑";
    duration = 0;
    playCount = 0;
    AudioPlayer::extractMetadata(p, title, artist, album, duration);
}
