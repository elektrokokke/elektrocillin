#ifndef GRAPHICSKEYITEM_H
#define GRAPHICSKEYITEM_H

/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

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

#include <QGraphicsPolygonItem>
#include <QBrush>
#include <QStack>

class GraphicsKeyItem : public QGraphicsPolygonItem
{
public:
    enum Type {
        BLACK,
        WHITE
    };
    GraphicsKeyItem(unsigned char noteNumber, const QSizeF &keySize, QGraphicsItem *parent = 0);

    Type getType() const;

    void pressKey();
    void releaseKey();

private:
    unsigned char noteNumber;
    QSizeF whiteKeySize, blackKeySize;
    QStack<QBrush> oldBrushes;
    Type style;

    QRectF getBoundingRectangle(unsigned char noteNumber, bool *isBlack = 0);
};

#endif // GRAPHICSKEYITEM_H
