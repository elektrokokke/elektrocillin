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

#include "graphicskeyitem.h"
#include <QPen>
#include <QGraphicsSceneMouseEvent>

GraphicsKeyItem::GraphicsKeyItem(unsigned char noteNumber_, const QSizeF &keySize, QGraphicsItem *parent) :
    QGraphicsPolygonItem(parent),
    noteNumber(noteNumber_),
    whiteKeySize(keySize),
    blackKeySize(keySize.width() * 0.6, keySize.height() * 0.65)
{
    // create a polygon representing a key:
    // create the bounding box:
    bool black;
    QPolygonF polygon = getBoundingRectangle(noteNumber, &black);
    if (!black) {
        // subtract the key left from it:
        if (noteNumber > 0) {
            polygon = polygon.subtracted(getBoundingRectangle(noteNumber - 1));
        }
        // subtract the key right from it:
        if (noteNumber < 127) {
            polygon = polygon.subtracted(getBoundingRectangle(noteNumber + 1));
        }
    }
    setPolygon(polygon);
    if (black) {
        setBrush(QBrush(Qt::black));
        setZValue(1);
    }
    setPen(QPen(Qt::gray));
    style = (black ? BLACK : WHITE);

    setAcceptHoverEvents(true);
}

GraphicsKeyItem::Type GraphicsKeyItem::getType() const
{
    return style;
}

void GraphicsKeyItem::pressKey()
{
    oldBrushes.push(brush());
    setBrush(QBrush(Qt::gray));
}

void GraphicsKeyItem::releaseKey()
{
    if (!oldBrushes.isEmpty()) {
        setBrush(oldBrushes.pop());
    }
}

QRectF GraphicsKeyItem::getBoundingRectangle(unsigned char noteNumber, bool *isBlack)
{
    int keyInOctave = noteNumber % 12;
    int octave = noteNumber / 12;
    qreal xOffset = whiteKeySize.width() * 7 * octave;
    if ((keyInOctave >= 0) && (keyInOctave <= 4)) {
        // first group of five keys:
        int index = keyInOctave / 2;
        bool black = (keyInOctave % 2);
        if (isBlack) {
            *isBlack = black;
        }
        if (black) {
            qreal x = xOffset + (index + 1) * (whiteKeySize.width() * 3 - blackKeySize.width() * 2) / 3 + index * blackKeySize.width();
            return QRectF(x, 0, blackKeySize.width(), blackKeySize.height());
        } else {
            qreal x = xOffset + index * whiteKeySize.width();
            return QRectF(x, 0, whiteKeySize.width(), whiteKeySize.height());
        }
    } else {
        xOffset += whiteKeySize.width() * 3;
        int index = (keyInOctave - 5) / 2;
        bool black = ((keyInOctave - 5) % 2);
        if (isBlack) {
            *isBlack = black;
        }
        if (black) {
            qreal x = xOffset + (index + 1) * (whiteKeySize.width() * 4 - blackKeySize.width() * 3) / 4 + index * blackKeySize.width();
            return QRectF(x, 0, blackKeySize.width(), blackKeySize.height());
        } else {
            qreal x = xOffset + index * whiteKeySize.width();
            return QRectF(x, 0, whiteKeySize.width(), whiteKeySize.height());
        }
    }
}
