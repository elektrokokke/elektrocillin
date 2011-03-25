#ifndef GRAPHICSCLIENTITEM2_H
#define GRAPHICSCLIENTITEM2_H

#include "jackclient.h"
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QGraphicsSimpleTextItem>
#include <QCursor>
#include <QBrush>
#include <QFont>
#include <QMenu>
#include <QGraphicsSceneMouseEvent>
#include <QPainterPathStroker>

class CommandTextItem;
class JackContextGraphicsScene;

class GraphicsClientItem : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
public:
    GraphicsClientItem(JackClient *client, int type, int portType, QFont font, QGraphicsItem *parent, JackContextGraphicsScene *scene);
    GraphicsClientItem(JackClient *client, const QString &clientName, int type, int portType, QFont font, QGraphicsItem *parent, JackContextGraphicsScene *scene);
    ~GraphicsClientItem();
    const QString & getClientName() const;
    const QRectF & getRect() const;
    void setInnerItem(QGraphicsItem *item);
    QGraphicsItem * getInnerItem() const;
public slots:
    void showInnerItem(bool ensureVisible = false);
    void updateBounds();
    void zoomToInnerItem();
    void onPortRegistered(QString fullPortName, QString type, int flags);

protected:
    void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
    void focusInEvent(QFocusEvent * event);
    void focusOutEvent(QFocusEvent * event);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
private slots:
    void onActionRemoveClient();
    void onActionEditMacro();
private:
    JackClient *client;
    QString clientName;
    int type, portType, padding;
    QFont font;
    QRectF rect;
    QGraphicsItem *innerItem;
    CommandTextItem *showInnerItemCommand, *zoomToInnerItemCommand;
    QPainterPath pathWithoutInnerItem;
    QMenu *contextMenu;
    bool isMacro;
    QAction *showInnerItemAction;
    QPainterPathStroker pathStroker;

    void initItem();
    void initRest();
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
        setBrush(QBrush(Qt::black));
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
            setBrush(QBrush(Qt::lightGray));
        }
    }
    void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event )
    {
        if (contains(event->pos())) {
            triggered();
        }
        setBrush(QBrush(Qt::black));
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
