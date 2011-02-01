#include "graphicskeyboarditem.h"
#include "graphicskeyitem.h"
#include <QGraphicsSceneMouseEvent>
#include <QPen>

GraphicsKeyboardItem::GraphicsKeyboardItem(unsigned char channel_, QGraphicsItem *parent) :
    QGraphicsRectItem(parent),
    channel(channel_),
    activeKey(0)
{
    QSizeF whiteKeySize(20, 100);
    // create keys:
    for (int i = 0; i < 128; i++) {
        keys.append(new GraphicsKeyItem(i, whiteKeySize, this));
    }
    setRect(childrenBoundingRect());
    setPen(QPen(Qt::NoPen));
}

unsigned char GraphicsKeyboardItem::getChannel() const
{
    return channel;
}

void GraphicsKeyboardItem::pressKey(unsigned char channel, unsigned char, unsigned char noteNumber)
{
    if (channel == getChannel()) {
        keys[noteNumber]->pressKey();
    }
}

void GraphicsKeyboardItem::releaseKey(unsigned char channel, unsigned char, unsigned char noteNumber)
{
    if (channel == getChannel()) {
        keys[noteNumber]->releaseKey();
    }
}

void GraphicsKeyboardItem::mousePressEvent ( QGraphicsSceneMouseEvent *event )
{
    Q_ASSERT(!activeKey);
    for (int i = 0; i < keys.size(); i++) {
        if (keys[i]->contains(event->pos())) {
            activeKey = keys[i];
            activeKey->pressKey();
            activeKeyIndex = i;
            keyPressed(getChannel(), 127, activeKeyIndex);
            return;
        }
    }
    event->ignore();
}

void GraphicsKeyboardItem::mouseReleaseEvent ( QGraphicsSceneMouseEvent * )
{
    if (activeKey) {
        activeKey->releaseKey();
        keyReleased(getChannel(), 127, activeKeyIndex);
        activeKey = 0;
    }
}

void GraphicsKeyboardItem::mouseMoveEvent ( QGraphicsSceneMouseEvent *event )
{
    if (activeKey && !activeKey->contains(event->pos())) {
        mouseReleaseEvent(event);
        mousePressEvent(event);
    } else if (!activeKey && contains(event->pos())) {
        mousePressEvent(event);
    }
}
