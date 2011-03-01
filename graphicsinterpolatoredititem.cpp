#include "graphicsinterpolatoredititem.h"
#include <QGraphicsSceneMouseEvent>
#include <QtGlobal>

GraphicsInterpolatorEditItem::GraphicsInterpolatorEditItem(Interpolator *interpolator_, const QRectF &rectangle, const QRectF &rectScaled, QGraphicsItem *parent, int verticalSlices_, int horizontalSlices_, const QPen &nodePen_, const QBrush &nodeBrush_) :
    QGraphicsRectItem(parent),
    interpolator(interpolator_),
    child(0),
    verticalSlices(qMax(1, verticalSlices_)),
    horizontalSlices(qMax(1, horizontalSlices_)),
    nodePen(nodePen_),
    nodeBrush(nodeBrush_)
{
    setPen(QPen(QBrush(Qt::black), 2));
    setBrush(QBrush(Qt::white));

    setRect(rectangle, rectScaled);
}

void GraphicsInterpolatorEditItem::setRect(const QRectF &rectangle, const QRectF &rectScaled)
{
    QGraphicsRectItem::setRect(rectangle);

    QList<QGraphicsItem*> children = childItems();
    for (int i = 0; i < children.size(); i++) {
        if (children[i] != child) {
            delete children[i];
        }
    }

    qreal tickSize = 10;
    // create the first horizontal textual label:
    QGraphicsSimpleTextItem *horizontalLabel = new QGraphicsSimpleTextItem(QString("%1").arg(rectScaled.bottom(), 0, 'g', 5), this);
    qreal padding = horizontalLabel->boundingRect().height();
    // compute the inner rectangle's bottom and top:
    qreal innerBottom = rect().bottom() - tickSize - padding * 2;
    qreal innerTop = rect().top() + padding;
    // now create the vertical labels, put them at zero and multiples of 10:
    qreal maxLabelWidth = horizontalLabel->boundingRect().width() * 0.5 - tickSize;
    for (int i = 0; i <= horizontalSlices; i++) {
        double value = (double)i * (rectScaled.top() - rectScaled.bottom()) / (double)horizontalSlices + rectScaled.bottom();
        // compute the vertical position of the label:
        double y = (double)i * (innerTop - innerBottom) / (double)horizontalSlices + innerBottom;
        // create a vertical label:
        QGraphicsSimpleTextItem *verticalLabel = new QGraphicsSimpleTextItem(QString("%1").arg(value, 0, 'g', 5), this);
        if (verticalLabel->boundingRect().width() > maxLabelWidth) {
            maxLabelWidth = verticalLabel->boundingRect().width();
        }
        // move it to the right position:
        verticalLabel->setPos(rect().left() + padding, y - padding * 0.5);
    }
    // compute the inner rectangle's left and right:
    qreal innerLeft = rect().left() + maxLabelWidth + padding + tickSize;
    qreal innerRight = rect().right() - padding;
    QRectF innerRectangle = QRectF(innerLeft, innerTop, innerRight - innerLeft, innerBottom - innerTop);
    // create the vertical ticks and dotted horizontal lines:
    for (int i = 0; i <= horizontalSlices; i++) {
        double y = (double)i * (innerTop - innerBottom) / (double)horizontalSlices + innerBottom;
        new QGraphicsLineItem(innerLeft - tickSize * 0.5, y, innerLeft, y, this);
        (new QGraphicsLineItem(innerLeft, y, innerRight, y, this))->setPen(QPen(Qt::DotLine));
    }
    // move the first horizontal label to the right position:
    horizontalLabel->setPos(innerLeft - horizontalLabel->boundingRect().width() * 0.5, innerBottom + tickSize);
    qreal lastRight = innerLeft + horizontalLabel->boundingRect().width() * 0.5;
    // create horizontal ticks and dotted vertical lines:
    for (int i = 0; i <= verticalSlices; i++) {
        double value = (double)i * (rectScaled.right() - rectScaled.left()) / (double)verticalSlices + rectScaled.left();
        double x = (double)i * (innerRight - innerLeft) / (double)verticalSlices + innerLeft;
        new QGraphicsLineItem(x, innerBottom, x, innerBottom + tickSize * 0.5, this);
        (new QGraphicsLineItem(x, innerTop, x, innerBottom, this))->setPen(QPen(Qt::DotLine));
        // create another horizontal label if it fits:
        QGraphicsSimpleTextItem *label = new QGraphicsSimpleTextItem(QString("%1").arg(value, 0, 'g', 5));
        if ((x - label->boundingRect().width() * 0.5 > lastRight) && (x + label->boundingRect().width() * 0.5 < rect().right())) {
            label->setParentItem(this);
            label->setPos(x - label->boundingRect().width() * 0.5, innerBottom + tickSize);
            lastRight = x + label->boundingRect().width() * 0.5;
        } else {
            delete label;
        }
    }

    if (child) {
        child->setRect(innerRectangle, rectScaled);
    } else {
        child = new GraphicsInterpolatorEditSubItem(interpolator, innerRectangle, rectScaled, this, nodePen, nodeBrush);
    }
}

void GraphicsInterpolatorEditItem::setVisible(ControlPoint controlPoint, bool visible)
{
    child->setVisible(controlPoint, visible);
}

void GraphicsInterpolatorEditItem::interpolatorChanged()
{
    child->interpolatorChanged();
}

Interpolator * GraphicsInterpolatorEditItem::getInterpolator()
{
    return child->getInterpolator();
}

GraphicsInterpolatorEditSubItem::GraphicsInterpolatorEditSubItem(Interpolator *interpolator_, const QRectF &rectangle, const QRectF &rectScaled_, GraphicsInterpolatorEditItem *parent_, const QPen &nodePen_, const QBrush &nodeBrush_) :
    QGraphicsRectItem(rectangle, parent_),
    rectScaled(rectScaled_),
    parent(parent_),
    nodePen(nodePen_),
    nodeBrush(nodeBrush_),
    interpolator(interpolator_)
{
    visible[GraphicsInterpolatorEditItem::FIRST] = visible[GraphicsInterpolatorEditItem::LAST] = true;
    setPen(QPen(Qt::NoPen));
    setBrush(QBrush(Qt::NoBrush));
    for (int i = 0; i < interpolator->getX().size(); i++) {
        nodes.append(createNode(interpolator->getX()[i], interpolator->interpolate(i, interpolator->getX()[i]), rectScaled));
        mapSenderToControlPointIndex[nodes.back()] = i;
    }
    nodes.first()->setVisible(visible[GraphicsInterpolatorEditItem::FIRST]);
    nodes.back()->setVisible(visible[GraphicsInterpolatorEditItem::LAST]);
    interpolationItem = new GraphicsInterpolationItem(interpolator, 0.01, rectScaled.bottom(), rectScaled.top(), rect().width() / rectScaled.width(), rect().height() / rectScaled.height(), this);
    interpolationItem->setPen(QPen(QBrush(Qt::black), 2));
    interpolationItem->setPos(-rectScaled.left() * rect().width() / rectScaled.width() + rect().left(), -rectScaled.top() * rect().height() / rectScaled.height() + rect().top());
    // create the context menu:
    contextMenu.addAction(tr("Increase nr. of control points"), this, SLOT(onIncreaseControlPoints()));
    contextMenu.addAction(tr("Decrease nr. of control points"), this, SLOT(onDecreaseControlPoints()));
}

void GraphicsInterpolatorEditSubItem::setRect(const QRectF &rect_, const QRectF &scaled_)
{
    QGraphicsRectItem::setRect(rect_);
    rectScaled = scaled_;
    // update the interpolation item accordingly:
    delete interpolationItem;
    interpolationItem = new GraphicsInterpolationItem(interpolator, 0.01, rectScaled.bottom(), rectScaled.top(), rect().width() / rectScaled.width(), rect().height() / rectScaled.height(), this);
    interpolationItem->setPen(QPen(QBrush(Qt::black), 2));
    interpolationItem->setPos(-rectScaled.left() * rect().width() / rectScaled.width() + rect().left(), -rectScaled.top() * rect().height() / rectScaled.height() + rect().top());
    for (int i = 0; i < nodes.size(); i++) {
        nodes[i]->setBounds(rect());
        nodes[i]->setBoundsScaled(rectScaled);
        nodes[i]->setXScaled(interpolator->getX()[i]);
        nodes[i]->setYScaled(interpolator->interpolate(i, interpolator->getX()[i]));
    }
}

void GraphicsInterpolatorEditSubItem::setVisible(GraphicsInterpolatorEditItem::ControlPoint controlPoint, bool visible)
{
    this->visible[controlPoint] = visible;
    if (controlPoint == GraphicsInterpolatorEditItem::FIRST) {
        nodes.first()->setVisible(visible);
    } else if (controlPoint == GraphicsInterpolatorEditItem::LAST) {
        nodes.back()->setVisible(visible);
    }
}

void GraphicsInterpolatorEditSubItem::interpolatorChanged()
{
    interpolationItem->updatePath();
    nodes.first()->setVisible(true);
    nodes.back()->setVisible(true);
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
    nodes.first()->setVisible(visible[GraphicsInterpolatorEditItem::FIRST]);
    nodes.back()->setVisible(visible[GraphicsInterpolatorEditItem::LAST]);
}

Interpolator * GraphicsInterpolatorEditSubItem::getInterpolator()
{
    return interpolator;
}

void GraphicsInterpolatorEditSubItem::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
    QGraphicsRectItem::mousePressEvent(event);
    if (!event->isAccepted() && (event->button() == Qt::RightButton)) {
        event->accept();
        // show a menu that allows removing and adding control points:
        contextMenu.exec(event->screenPos());
    }
}

void GraphicsInterpolatorEditSubItem::onIncreaseControlPoints()
{
    // signal the control point increase event and update our interpolator graphic item:
    parent->increaseControlPoints();
    interpolatorChanged();
}

void GraphicsInterpolatorEditSubItem::onDecreaseControlPoints()
{
    // signal the control point decrease event and update our interpolator graphic item:
    parent->decreaseControlPoints();
    interpolatorChanged();
}

void GraphicsInterpolatorEditSubItem::onNodePositionChangedScaled(QPointF position)
{
    // get the control point index:
    int index = mapSenderToControlPointIndex[sender()];
    // signal the control point change event and update our interpolator graphic item:
    parent->changeControlPoint(index, position.x(), position.y());
    interpolationItem->updatePath();
}

GraphicsNodeItem * GraphicsInterpolatorEditSubItem::createNode(qreal x, qreal y, const QRectF &rectScaled)
{
    GraphicsNodeItem *nodeItem = new GraphicsNodeItem(-5.0, -5.0, 10.0, 10.0, this);
    nodeItem->setPen(nodePen);
    nodeItem->setBrush(nodeBrush);
    nodeItem->setZValue(1);
    nodeItem->setBounds(QRectF(rect().left(), rect().top(), rect().width(), rect().height()));
    nodeItem->setBoundsScaled(rectScaled);
    nodeItem->setXScaled(x);
    nodeItem->setYScaled(y);
    nodeItem->setSendPositionChanges(true);
    QObject::connect(nodeItem, SIGNAL(positionChangedScaled(QPointF)), this, SLOT(onNodePositionChangedScaled(QPointF)));
    return nodeItem;
}
