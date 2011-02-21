#include "graphicsinterpolatoredititem.h"
#include <QGraphicsSceneMouseEvent>
#include <QPen>

GraphicsInterpolatorEditItem::GraphicsInterpolatorEditItem(Interpolator *interpolator_, const QRectF &rectangle, const QRectF &rectScaled_, QGraphicsItem *parent) :
    QGraphicsRectItem(rectangle, parent),
    rectScaled(rectScaled_),
    interpolator(interpolator_)
{
    setPen(QPen(QBrush(Qt::black), 2));
    setBrush(QBrush(Qt::white));
    // create dotted vertical lines:
    for (int i = 0; i <= 8; i++) {
        qreal x = (double)i / 8.0 * (rect().right() - rect().left()) + rect().left();
        (new QGraphicsLineItem(x, rect().top(), x, rect().bottom(), this))->setPen(QPen(Qt::DotLine));
    }
    // create dotted horizontal lines:
    for (int i = 0; i <= 8; i++) {
        qreal y = (double)i / 8.0 * (rect().top() - rect().bottom()) + rect().bottom();
        (new QGraphicsLineItem(rect().left(), y, rect().right(), y, this))->setPen(QPen(Qt::DotLine));
    }
    for (int i = 0; i < interpolator->getX().size(); i++) {
        nodes.append(createNode(interpolator->getX()[i], interpolator->interpolate(i, interpolator->getX()[i]), rectScaled));
        mapSenderToControlPointIndex[nodes.back()] = i;
    }
    interpolationItem = new GraphicsInterpolationItem(interpolator, 0.01, rectScaled.bottom(), rectScaled.top(), rect().width() / rectScaled.width(), rect().height() / rectScaled.height(), this);
    interpolationItem->setPen(QPen(QBrush(Qt::black), 2));
    interpolationItem->setPos(-rectScaled.left() * rect().width() / rectScaled.width() + rect().left(), -rectScaled.top() * rect().height() / rectScaled.height() + rect().top());
    // create the context menu:
    contextMenu.addAction(tr("Increase nr. of control points"), this, SLOT(onIncreaseControlPoints()));
    contextMenu.addAction(tr("Decrease nr. of control points"), this, SLOT(onDecreaseControlPoints()));
}

void GraphicsInterpolatorEditItem::interpolatorChanged()
{
    interpolationItem->updatePath();
    for (; nodes.size() > interpolator->getX().size(); ) {
        mapSenderToControlPointIndex.remove(nodes.back());
        delete nodes.back();
        nodes.remove(nodes.size() - 1);
    }
    for (int i = 0; i < nodes.size(); i++) {
        nodes[i]->setXScaled(interpolator->getX()[i]);
        nodes[i]->setYScaled(interpolator->interpolate(i, interpolator->getX()[i]));
        mapSenderToControlPointIndex[nodes[i]] = i;
    }
    for (int i = nodes.size(); i < interpolator->getX().size(); i++) {
        // add a new node:
        nodes.append(createNode(interpolator->getX()[i], interpolator->interpolate(i, interpolator->getX()[i]), rectScaled));
        mapSenderToControlPointIndex[nodes[i]] = i;
    }
}

void GraphicsInterpolatorEditItem::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
    QGraphicsRectItem::mousePressEvent(event);
    if (!event->isAccepted() && (event->button() == Qt::RightButton)) {
        event->accept();
        // show a menu that allows removing and adding control points:
        contextMenu.exec(event->screenPos());
    }
}

void GraphicsInterpolatorEditItem::onIncreaseControlPoints()
{
    // signal the control point increase event and update our interpolator graphic item:
    increaseControlPoints();
    interpolationItem->updatePath();
    // update the existing nodes:
    for (int i = 0; i < nodes.size(); i++) {
        nodes[i]->setXScaled(interpolator->getX()[i]);
        nodes[i]->setYScaled(interpolator->interpolate(i, interpolator->getX()[i]));
        mapSenderToControlPointIndex[nodes[i]] = i;
    }
    for (int i = nodes.size(); i < interpolator->getX().size(); i++) {
        // add a new node:
        nodes.append(createNode(interpolator->getX()[i], interpolator->interpolate(i, interpolator->getX()[i]), rectScaled));
        mapSenderToControlPointIndex[nodes[i]] = i;
    }
}

void GraphicsInterpolatorEditItem::onDecreaseControlPoints()
{
    // signal the control point decrease event and update our interpolator graphic item:
    decreaseControlPoints();
    interpolationItem->updatePath();
    // remove the superfluous nodes from our node map and delete them:
    for (; nodes.size() > interpolator->getX().size(); ) {
        mapSenderToControlPointIndex.remove(nodes.back());
        delete nodes.back();
        nodes.remove(nodes.size() - 1);
    }
    // update the remaining nodes:
    for (int i = 0; i < nodes.size(); i++) {
        nodes[i]->setXScaled(interpolator->getX()[i]);
        nodes[i]->setYScaled(interpolator->interpolate(i, interpolator->getX()[i]));
        mapSenderToControlPointIndex[nodes[i]] = i;
    }
}

void GraphicsInterpolatorEditItem::onNodePositionChangedScaled(QPointF position)
{
    // get the control point index:
    int index = mapSenderToControlPointIndex[sender()];
    // signal the control point change event and update our interpolator graphic item:
    changeControlPoint(index, interpolator->getX().size(), position.x(), position.y());
    interpolationItem->updatePath();
}

GraphicsNodeItem * GraphicsInterpolatorEditItem::createNode(qreal x, qreal y, const QRectF &rectScaled)
{
    GraphicsNodeItem *nodeItem = new GraphicsNodeItem(-5.0, -5.0, 10.0, 10.0, this);
    nodeItem->setPen(QPen(QBrush(qRgb(114, 159, 207)), 3));
    nodeItem->setBrush(QBrush(qRgb(52, 101, 164)));
    nodeItem->setZValue(1);
    nodeItem->setBounds(QRectF(rect().left(), rect().top(), rect().width(), rect().height()));
    nodeItem->setBoundsScaled(rectScaled);
    nodeItem->setXScaled(x);
    nodeItem->setYScaled(y);
    nodeItem->setSendPositionChanges(true);
    QObject::connect(nodeItem, SIGNAL(positionChangedScaled(QPointF)), this, SLOT(onNodePositionChangedScaled(QPointF)));
    return nodeItem;
}
