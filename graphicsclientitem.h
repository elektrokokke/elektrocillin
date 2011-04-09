#ifndef GRAPHICSCLIENTITEM2_H
#define GRAPHICSCLIENTITEM2_H

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

#include "jackclient.h"
#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QPainterPath>
#include <QGraphicsSimpleTextItem>
#include <QCursor>
#include <QBrush>
#include <QFont>
#include <QMenu>
#include <QGraphicsSceneMouseEvent>
#include <QPainterPathStroker>
#include <QPointer>

class CommandTextItem;
class GraphicsClientItemsClient;

class GraphicsClientItem : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:
    GraphicsClientItem(GraphicsClientItemsClient *clientItemsClient, JackClient *jackClient, bool isMacro, const QString &clientName, int style, int audioPortStyle, int midiPortStyle, QFont font, QGraphicsItem *parent);
    const QString & getClientName() const;
    const QRectF & getRect() const;

    QGraphicsItem * getControlsItem() const;
    bool isControlsVisible() const;
    void setControlsVisible(bool visible);

    bool isMacroItem() const;
    bool isModuleItem() const;
public slots:
    void toggleControls(bool ensureVisible = false);
    void updatePorts();
protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
    virtual void focusInEvent(QFocusEvent * event);
private:
    GraphicsClientItemsClient *clientItemsClient;
    QPointer<JackClient> jackClient;
    QString clientName;
    int clientStyle, audioPortStyle, midiPortStyle;
    QFont font;
    QRectF rect;
    QGraphicsItem *controlsItem;
    bool isMacro;

    void initItem();
};

class RectanglePath : public QPainterPath
{
public:
    RectanglePath(const QRectF &rect)
    {
        addRect(rect);
    }
};

class RoundedRectanglePath : public QPainterPath
{
public:
    RoundedRectanglePath(const QRectF &rect, qreal xRadius, qreal yRadius, Qt::SizeMode mode = Qt::AbsoluteSize)
    {
        addRoundedRect(rect, xRadius, yRadius, mode);
    }
};

class EllipsePath : public QPainterPath
{
public:
    EllipsePath(const QRectF &rect)
    {
        addEllipse(rect);
    }
};

class SpeechBubblePath : public QPainterPath
{
public:
    SpeechBubblePath(const QRectF &rect, qreal xRadius, qreal yRadius, Qt::SizeMode mode = Qt::RelativeSize)
    {
        QPointF topCenter(rect.center().x(), rect.top());
        QPointF bottomCenter(rect.center().x(), rect.bottom());
        QPointF leftCenter(rect.left(), rect.center().y());
        QPointF rightCenter(rect.right(), rect.center().y());
        moveTo(topCenter);
        if (mode == Qt::RelativeSize) {
            cubicTo((1.0 - xRadius) * topCenter + xRadius * rect.topRight(), (1.0 - yRadius) * rightCenter + yRadius * rect.topRight(), rightCenter);
            cubicTo((1.0 - yRadius) * rightCenter + yRadius * rect.bottomRight(), (1.0 - xRadius) * bottomCenter + xRadius * rect.bottomRight(), bottomCenter);
            cubicTo((1.0 - xRadius) * bottomCenter + xRadius * rect.bottomLeft(), (1.0 - yRadius) * leftCenter + yRadius * rect.bottomLeft(), leftCenter);
            cubicTo((1.0 - yRadius) * leftCenter + yRadius * rect.topLeft(), (1.0 - xRadius) * topCenter + xRadius * rect.topLeft(), topCenter);
        } else {
            cubicTo(rect.topRight() + QPointF(-xRadius, 0), rect.topRight() + QPointF(0, yRadius), rightCenter);
            cubicTo(rect.bottomRight() + QPointF(0, -yRadius), rect.bottomRight() + QPointF(-xRadius, 0), bottomCenter);
            cubicTo(rect.bottomLeft() + QPointF(xRadius, 0), rect.bottomLeft() + QPointF(0, -yRadius), leftCenter);
            cubicTo(rect.topLeft() + QPointF(0, yRadius), rect.topLeft() + QPointF(xRadius, 0), topCenter);
        }
    }
};

#endif // GRAPHICSCLIENTITEM2_H
