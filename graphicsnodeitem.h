#ifndef GRAPHICSNODEITEM_H
#define GRAPHICSNODEITEM_H

#include <QObject>
#include <QGraphicsEllipseItem>

#include "graphicslineitem.h"
#include "graphicsloglineitem.h"

class GraphicsNodeItem : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT
public:
    explicit GraphicsNodeItem( QGraphicsItem * parent = 0 );
    explicit GraphicsNodeItem( const QRectF & rect, QGraphicsItem * parent = 0 );
    explicit GraphicsNodeItem( qreal x, qreal y, qreal width, qreal height, QGraphicsItem * parent = 0 );

    void setBounds(const QRectF &bounds);
    void resetBounds();

    void connectLine(GraphicsLineItem *line, GraphicsLineItem::EndPoints endPoint);
    void connectLine(GraphicsLogLineItem *line, GraphicsLineItem::EndPoints endPoint);

signals:
    void positionChanged(QPointF position);
    void xChanged(qreal x);
    void yChanged(qreal y);

public slots:
    void setX(qreal x);
    void setY(qreal y);
    void setBoundsLeft(qreal left);
    void setBoundsRight(qreal right);

protected:
    virtual void hoverEnterEvent( QGraphicsSceneHoverEvent * event );
    virtual void hoverLeaveEvent( QGraphicsSceneHoverEvent * event );
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    void init();
    QPointF adjustToBounds(const QPointF &point);
    QRectF bounds, oldRect;
    bool considerBounds, changingCoordinates;

};

#endif // GRAPHICSNODEITEM_H
