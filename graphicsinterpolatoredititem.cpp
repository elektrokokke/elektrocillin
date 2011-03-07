#include "graphicsinterpolatoredititem.h"
#include <QGraphicsSceneMouseEvent>
#include <QtGlobal>
#include <cmath>

GraphicsInterpolatorEditItem::GraphicsInterpolatorEditItem(Interpolator *interpolator_, const QRectF &rectangle, const QRectF &rectScaled, QGraphicsItem *parent, int verticalSlices_, int horizontalSlices_, bool logarithmicX_, const QPen &nodePen_, const QBrush &nodeBrush_) :
    QGraphicsRectItem(parent),
    interpolator(interpolator_),
    child(0),
    verticalSlices(qMax(1, verticalSlices_)),
    horizontalSlices(qMax(1, horizontalSlices_)),
    logarithmicX(logarithmicX_),
    nodePen(nodePen_),
    nodeBrush(nodeBrush_)
{
    setPen(QPen(QBrush(Qt::black), 2));
    setBrush(QBrush(Qt::white));
    font.setPointSize(6);

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
    double t = (logarithmicX ? exp(rectScaled.left()) - 1 : rectScaled.left());
    QGraphicsSimpleTextItem *horizontalLabel = new QGraphicsSimpleTextItem(QString("%1").arg(t, 0, 'g', 3), this);
    horizontalLabel->setFont(font);
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
        QGraphicsSimpleTextItem *verticalLabel = new QGraphicsSimpleTextItem(QString("%1").arg(value, 0, 'g', 3), this);
        verticalLabel->setFont(font);
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
        double value1 = (double)i * (rectScaled.right() - rectScaled.left()) / (double)verticalSlices + rectScaled.left();
        double t1 = (logarithmicX ? exp(value1) - 1 : value1);
        double x1 = (double)i * (innerRight - innerLeft) / (double)verticalSlices + innerLeft;
        new QGraphicsLineItem(x1, innerBottom, x1, innerBottom + tickSize * 0.5, this);
        (new QGraphicsLineItem(x1, innerTop, x1, innerBottom, this))->setPen(QPen(Qt::DotLine));
        // create another horizontal label if it fits:
        QGraphicsSimpleTextItem *label1 = new QGraphicsSimpleTextItem(QString("%1").arg(t1, 0, 'g', 3));
        label1->setFont(font);
        if ((x1 - label1->boundingRect().width() * 0.5 > lastRight) && (x1 + label1->boundingRect().width() * 0.5 < rect().right())) {
            label1->setParentItem(this);
            label1->setPos(x1 - label1->boundingRect().width() * 0.5, innerBottom + tickSize);
            lastRight = x1 + label1->boundingRect().width() * 0.5;
        } else {
            delete label1;
        }
        if (logarithmicX && (i < verticalSlices)) {
            double value2 = (double)(i + 1) * (rectScaled.right() - rectScaled.left()) / (double)verticalSlices + rectScaled.left();
            double t2 = exp(value2) - 1;
            double x2 = (double)(i + 1) * (innerRight - innerLeft) / (double)verticalSlices + innerLeft;
            QGraphicsSimpleTextItem *label2 = new QGraphicsSimpleTextItem(QString("%1").arg(t2, 0, 'g', 3));
            label2->setFont(font);
            label2->setPos(x2 - label2->boundingRect().width() * 0.5, innerBottom + tickSize);
            qreal nextLeft = x2 - label2->boundingRect().width() * 0.5;
            delete label2;
            for (int j = 1; j < 16; j++) {
                double t = (double)j / 16.0 * (t2 - t1) + t1;
                double value = log(t + 1);
                double x = value * (innerRight - innerLeft) / (rectScaled.right() - rectScaled.left()) + innerLeft;
                new QGraphicsLineItem(x, innerBottom, x, innerBottom + tickSize * 0.5, this);
                (new QGraphicsLineItem(x, innerTop, x, innerBottom, this))->setPen(QPen(Qt::DotLine));
                // create another horizontal label if it fits:
                QGraphicsSimpleTextItem *label = new QGraphicsSimpleTextItem(QString("%1").arg(t, 0, 'g', 3));
                label->setFont(font);
                if ((x - label->boundingRect().width() * 0.5 > lastRight) && (x + label->boundingRect().width() * 0.5 < nextLeft)) {
                    label->setParentItem(this);
                    label->setPos(x - label->boundingRect().width() * 0.5, innerBottom + tickSize);
                    lastRight = x + label->boundingRect().width() * 0.5;
                } else {
                    delete label;
                }
            }
        }
    }

    if (child) {
        child->setRect(innerRectangle, rectScaled);
    } else {
        child = new GraphicsInterpolatorEditSubItem(interpolator, innerRectangle, rectScaled, this, logarithmicX, nodePen, nodeBrush);
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

QRectF GraphicsInterpolatorEditItem::getInnerRectangle() const
{
    return child->rect();
}

GraphicsInterpolatorEditSubItem::GraphicsInterpolatorEditSubItem(Interpolator *interpolator_, const QRectF &rectangle, const QRectF &rectScaled_, GraphicsInterpolatorEditItem *parent_, bool logarithmicX_, const QPen &nodePen_, const QBrush &nodeBrush_) :
    QGraphicsRectItem(rectangle, parent_),
    rectScaled(rectScaled_),
    parent(parent_),
    nodePen(nodePen_),
    nodeBrush(nodeBrush_),
    interpolator(interpolator_),
    logarithmicX(logarithmicX_)
{
    font.setPointSize(8);
    visible[GraphicsInterpolatorEditItem::FIRST] = visible[GraphicsInterpolatorEditItem::LAST] = true;
    setPen(QPen(Qt::NoPen));
    setBrush(QBrush(Qt::NoBrush));
    interpolationItem = new GraphicsInterpolationItem(interpolator, 0.01, rectScaled.bottom(), rectScaled.top(), rect().width() / rectScaled.width(), rect().height() / rectScaled.height(), this);
    interpolationItem->setPen(QPen(QBrush(Qt::black), 2));
    interpolationItem->setPos(-rectScaled.left() * rect().width() / rectScaled.width() + rect().left(), -rectScaled.top() * rect().height() / rectScaled.height() + rect().top());

    interpolatorChanged();

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
    if (nodes.size()) {
        nodes.first()->setVisible(true);
        nodes.back()->setVisible(true);
    }
    for (; nodes.size() > interpolator->getX().size(); ) {
        delete nodes.back();
        nodes.remove(nodes.size() - 1);
        delete nodesText.back();
        nodesText.remove(nodesText.size() - 1);
    }
    mapSenderToControlPointIndex.clear();
    for (int i = 0; i < nodes.size(); i++) {
        double x = interpolator->getX()[i];
        double y = interpolator->interpolate(i, x);
        nodes[i]->setXScaled(x);
        nodes[i]->setYScaled(y);
        QGraphicsSimpleTextItem *text = nodesText[i];
        if (logarithmicX) {
            text->setText(QString("%1, %2").arg(exp(x) - 1).arg(y));
        } else {
            text->setText(QString("%1, %2").arg(x).arg(y));
        }
        text->setPos(nodes[i]->pos() + QPointF(8, -text->boundingRect().height() * 0.5));
        mapSenderToControlPointIndex[nodes[i]] = i;
    }
    for (int i = nodes.size(); i < interpolator->getX().size(); i++) {
        // add a new node:
        double x = interpolator->getX()[i];
        double y = interpolator->interpolate(i, x);
        nodes.append(createNode(x, y, rectScaled));
        QGraphicsSimpleTextItem *text = new QGraphicsSimpleTextItem(this);
        text->setFont(font);
        if (logarithmicX) {
            text->setText(QString("%1, %2").arg(exp(x) - 1).arg(y));
        } else {
            text->setText(QString("%1, %2").arg(x).arg(y));
        }text->setPos(nodes[i]->pos() + QPointF(8, -text->boundingRect().height() * 0.5));
        nodesText.append(text);
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
//    QGraphicsSimpleTextItem *text = nodesText[index];
//    if (logarithmicX) {
//        text->setText(QString("%1, %2").arg(exp(position.x()) - 1).arg(position.y()));
//    } else {
//        text->setText(QString("%1, %2").arg(position.x()).arg(position.y()));
//    }
//    text->setPos(nodes[index]->pos() + QPointF(8, -text->boundingRect().height() * 0.3));
//    interpolationItem->updatePath();
    interpolatorChanged();
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
