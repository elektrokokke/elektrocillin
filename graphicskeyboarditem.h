#ifndef GRAPHICSKEYBOARDITEM_H
#define GRAPHICSKEYBOARDITEM_H

/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Elektrocillin is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Elektrocillin.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QGraphicsRectItem>
#include <QVector>

class GraphicsKeyItem;

class GraphicsKeyboardItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    explicit GraphicsKeyboardItem(unsigned char channel, QGraphicsItem *parent = 0);

    unsigned char getChannel() const;

signals:
    void keyPressed(unsigned char channel, unsigned char noteNumber, unsigned char velocity);
    void keyReleased(unsigned char channel, unsigned char noteNumber, unsigned char velocity);

public slots:
    void pressKey(unsigned char channel, unsigned char noteNumber, unsigned char velocity);
    void releaseKey(unsigned char channel, unsigned char noteNumber, unsigned char velocity);

protected:
    virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    virtual void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
    virtual void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );

private:
    QVector<GraphicsKeyItem*> keys;
    unsigned char channel;
    GraphicsKeyItem *activeKey;
    int activeKeyIndex;
};

#endif // GRAPHICSKEYBOARDITEM_H
