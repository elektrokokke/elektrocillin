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
    enum Scale {
        LINEAR,
        LOGARITHMIC
    };

    explicit GraphicsNodeItem( QGraphicsItem * parent = 0 );
    explicit GraphicsNodeItem( const QRectF & rect, QGraphicsItem * parent = 0 );
    explicit GraphicsNodeItem( qreal x, qreal y, qreal width, qreal height, QGraphicsItem * parent = 0 );

    void setSendPositionChanges(bool send);
    bool getSendPositionChanges() const;
    void setBounds(const QRectF &bounds);
    void resetBounds();
    bool getSendPositionChangesScaled() const;
    void setScale(Scale horizontal, Scale vertical);
    void setBoundsScaled(const QRectF &bounds);
    void resetBoundsScaled();

    void connectLine(GraphicsLineItem *line, GraphicsLineItem::EndPoints endPoint);
    void connectLine(GraphicsLogLineItem *line, GraphicsLineItem::EndPoints endPoint);

signals:
    void positionChanged(QPointF position);
    void xChanged(qreal x);
    void yChanged(qreal y);
    void positionChangedScaled(QPointF position);
    void xChangedScaled(qreal x);
    void yChangedScaled(qreal y);

public slots:
    void setX(qreal x);
    void setY(qreal y);
    void setXScaled(qreal xScaled);
    void setYScaled(qreal yScaled);
    void setBoundsLeft(qreal left);
    void setBoundsRight(qreal right);

protected:
    virtual void hoverEnterEvent( QGraphicsSceneHoverEvent * event );
    virtual void hoverLeaveEvent( QGraphicsSceneHoverEvent * event );
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    void init();
    QPointF adjustToBounds(const QPointF &point);
    QRectF bounds, boundsScaled, oldRect;
    Scale horizontalScale, verticalScale;
    bool considerBounds, considerBoundsScaled, sendPositionChanges, changingCoordinates;

    QPointF scale(const QPointF &p);
    qreal scaleX(qreal x);
    qreal scaleY(qreal y);
    QPointF descale(const QPointF &p);
    qreal descaleX(qreal x);
    qreal descaleY(qreal y);
};

#endif // GRAPHICSNODEITEM_H
