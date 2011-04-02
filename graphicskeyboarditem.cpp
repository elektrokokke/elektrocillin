/*
    Copyright 2011 Arne Jacobs

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "graphicskeyboarditem.h"
#include "graphicskeyitem.h"
#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include <QCursor>

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
    setBrush(QBrush(Qt::white));
    setCursor(Qt::ArrowCursor);
}

unsigned char GraphicsKeyboardItem::getChannel() const
{
    return channel;
}

void GraphicsKeyboardItem::pressKey(unsigned char channel, unsigned char noteNumber, unsigned char)
{
    if (channel == getChannel()) {
        keys[noteNumber]->pressKey();
    }
}

void GraphicsKeyboardItem::releaseKey(unsigned char channel, unsigned char noteNumber, unsigned char)
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
            keyPressed(getChannel(), activeKeyIndex, 127);
            return;
        }
    }
    event->ignore();
}

void GraphicsKeyboardItem::mouseReleaseEvent ( QGraphicsSceneMouseEvent * )
{
    if (activeKey) {
        activeKey->releaseKey();
        keyReleased(getChannel(), activeKeyIndex, 127);
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
