#ifndef PLAYLISTITEM_H
#define PLAYLISTITEM_H

#include <QString>
#include <QDateTime>

struct PlaylistItem {
    QString path;
    QString title;
    QString artist;
    QString album;
    qint64 duration = 0;
    int playCount = 0;
    QDateTime addedTime;
    
    PlaylistItem() {}
    PlaylistItem(const QString &p);
    
    bool operator<(const PlaylistItem &other) const {
        return path < other.path;
    }
};

#endif // PLAYLISTITEM_H
