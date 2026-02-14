#include "PlaylistDelegate.h"
#include <QPainter>
#include <QStyleOptionViewItem>

void PlaylistDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    Q_UNUSED(index);
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    painter->save();
    
    // 绘制背景
    if (opt.state & QStyle::State_Selected) {
        painter->fillRect(opt.rect, QColor("#e5f1fb")); 
    } else if (opt.state & QStyle::State_MouseOver) {
        painter->fillRect(opt.rect, QColor("#f5f5f5"));
    }

    QString fullText = opt.text;
    QStringList parts = fullText.split("    ");
    
    QRect rect = opt.rect.adjusted(10, 0, -10, 0);
    
    if (parts.size() >= 2) {
        QString serial = parts[0];
        QString content = parts[1];

        // 1. 绘制序号（淡灰色）
        painter->setPen(QColor("#999999"));
        QFont serialFont = painter->font();
        serialFont.setPointSize(9);
        painter->setFont(serialFont);
        
        int serialWidth = painter->fontMetrics().horizontalAdvance(serial + "    ");
        painter->drawText(rect, Qt::AlignVCenter | Qt::AlignLeft, serial);

        // 2. 绘制内容
        painter->setPen(opt.font.bold() ? QColor("#0078d7") : QColor("#222222"));
        QFont contentFont = painter->font();
        if (opt.font.bold()) contentFont.setBold(true);
        painter->setFont(contentFont);
        
        QRect contentRect = rect.adjusted(serialWidth, 0, 0, 0);
        painter->drawText(contentRect, Qt::AlignVCenter | Qt::AlignLeft, content);
    } else {
        painter->drawText(rect, Qt::AlignVCenter | Qt::AlignLeft, fullText);
    }

    painter->restore();
}

QSize PlaylistDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(option.rect.width(), 32);
}
