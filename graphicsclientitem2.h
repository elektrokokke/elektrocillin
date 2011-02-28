#ifndef GRAPHICSCLIENTITEM2_H
#define GRAPHICSCLIENTITEM2_H

#include "jackclient.h"
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QGraphicsSimpleTextItem>
#include <QCursor>
#include <QFont>

class GraphicsClientItem2 : public QGraphicsPathItem
{
public:
    GraphicsClientItem2(JackClient *client, int type, int portType, QGraphicsItem *parent = 0);
    GraphicsClientItem2(JackClient *client, const QString &clientName, int type, int portType, QGraphicsItem *parent = 0);
    const QString & getClientName() const;
    const QRectF & getRect() const;
private:
    JackClient *client;
    QString clientName;
    int type, portType;
    QRectF rect;

    void init();
};

class CommandTextItem : public QGraphicsSimpleTextItem
{
public:
    CommandTextItem(const QString &commandText, QGraphicsItem *parent = 0) :
        QGraphicsSimpleTextItem(commandText, parent)
    {
        QFont font("Mighty Zeo 2.0", 12);
        font.setStyleStrategy(QFont::PreferAntialias);
//        font.setBold(true);
        setFont(font);
        setCursor(Qt::PointingHandCursor);
    }
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
