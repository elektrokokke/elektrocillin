#ifndef GRAPHICSCLIENTITEM2_H
#define GRAPHICSCLIENTITEM2_H

#include "jackclient.h"
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QGraphicsSimpleTextItem>
#include <QCursor>
#include <QFont>
#include <QGraphicsSceneMouseEvent>

class CommandTextItem;

class GraphicsClientItem2 :public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:
    GraphicsClientItem2(JackClient *client, int type, int portType, QFont font, QGraphicsItem *parent = 0);
    GraphicsClientItem2(JackClient *client, const QString &clientName, int type, int portType, QFont font, QGraphicsItem *parent = 0);
    const QString & getClientName() const;
    const QRectF & getRect() const;
    void setInnerItem(QGraphicsItem *item);
    QGraphicsItem * getInnerItem() const;
public slots:
    void showInnerItem(bool ensureVisible = false);
protected:
    void focusInEvent(QFocusEvent * event);
    void focusOutEvent(QFocusEvent * event);
private:
    JackClient *client;
    QString clientName;
    int type, portType;
    QFont font;
    QRectF rect;
    QGraphicsItem *innerItem;
    CommandTextItem *showInnerItemCommand;
    QPainterPath pathWithoutInnerItem;

    void init();
};

class CommandTextItem : public QObject, public QGraphicsSimpleTextItem
{
    Q_OBJECT
public:
    CommandTextItem(const QString &commandText, QFont font, QGraphicsItem *parent = 0) :
        QGraphicsSimpleTextItem(commandText, parent)
    {
        font.setStyleStrategy(QFont::PreferAntialias);
        setFont(font);
        setCursor(Qt::PointingHandCursor);
    }
signals:
    void triggered();
protected:
    void mousePressEvent ( QGraphicsSceneMouseEvent * event )
    {
        QGraphicsSimpleTextItem::mousePressEvent(event);
        if (!event->isAccepted() && (event->button() == Qt::LeftButton)) {
            event->accept();
            triggered();
        }
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
