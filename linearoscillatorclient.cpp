#include "linearoscillatorclient.h"
#include "graphicsnodeitem.h"
#include <cmath>
#include <QPen>

LinearOscillatorClient::LinearOscillatorClient(const QString &clientName, size_t ringBufferSize) :
        EventProcessorClient<LinearOscillatorParameters>(clientName, new LinearOscillator(), ringBufferSize),
        interpolator(getLinearOscillator()->getLinearInterpolator())
{
}

LinearOscillatorClient::~LinearOscillatorClient()
{
    close();
}

LinearOscillator * LinearOscillatorClient::getLinearOscillator()
{
    return (LinearOscillator*)getMidiProcessor();
}

const LinearInterpolator & LinearOscillatorClient::postIncreaseControlPoints()
{
    int size = interpolator.getX().size() + 1;
    double stretchFactor = (double)(interpolator.getX().size() - 1) / (double)(size - 1);
    interpolator.getX().append(2 * M_PI);
    interpolator.getY().append(1);
    QVector<LinearOscillatorParameters> parameterVector;
    for (int i = 0; i < size; i++) {
        if (i < size - 1) {
            interpolator.getX()[i] = interpolator.getX()[i] * stretchFactor;
        }
        LinearOscillatorParameters parameters;
        parameters.controlPoints = size;
        parameters.index = i;
        parameters.x = interpolator.getX()[i];
        parameters.y = interpolator.getY()[i];
        parameterVector.append(parameters);
    }
    postEvents(parameterVector);
    return interpolator;
}

const LinearInterpolator & LinearOscillatorClient::postDecreaseControlPoints()
{
    if (interpolator.getX().size() > 2) {
        int size = interpolator.getX().size() - 1;
        interpolator.getX().resize(size);
        interpolator.getY().resize(size);
        double stretchFactor = 2 * M_PI / interpolator.getX().back();
        QVector<LinearOscillatorParameters> parameterVector;
        for (int i = 0; i < size; i++) {
            interpolator.getX()[i] = interpolator.getX()[i] * stretchFactor;
            LinearOscillatorParameters parameters;
            parameters.controlPoints = size;
            parameters.index = i;
            parameters.x = interpolator.getX()[i];
            parameters.y = interpolator.getY()[i];
            parameterVector.append(parameters);
        }
        postEvents(parameterVector);
    }
    return interpolator;
}

void LinearOscillatorClient::processEvent(const LinearOscillatorParameters &event, jack_nframes_t)
{
    // set the interpolator's nr of control points:
    interpolator.getX().resize(event.controlPoints);
    interpolator.getY().resize(event.controlPoints);
    // set the interpolator control point at "index" accordingly:
    interpolator.getX()[event.index] = event.x;
    interpolator.getY()[event.index] = event.y;
    // update the integral:
    getLinearOscillator()->setLinearInterpolator(interpolator);
}

LinearOscillatorGraphicsItem::LinearOscillatorGraphicsItem(const QRectF &rect, LinearOscillatorClient *client_, QGraphicsItem *parent) :
    QGraphicsRectItem(rect, parent),
    client(client_),
    interpolator(client->getLinearOscillator()->getLinearInterpolator()),
    interpolatorIntegral(client->getLinearOscillator()->getLinearIntegralInterpolator())
{
    setPen(QPen(QBrush(Qt::black), 2));
    setBrush(QBrush(Qt::white));
    // create dotted vertical lines:
    for (int i = 0; i <= 10; i++) {
        qreal x = (double)i / 10.0 * (rect.right() - rect.left()) + rect.left();
        (new QGraphicsLineItem(x, rect.top(), x, rect.bottom(), this))->setPen(QPen(Qt::DotLine));
    }
    // create dotted horizontal lines:
    for (int i = 0; i <= 10; i++) {
        qreal y = (double)i / 10.0 * (rect.top() - rect.bottom()) + rect.bottom();
        (new QGraphicsLineItem(rect.left(), y, rect.right(), y, this))->setPen(QPen(Qt::DotLine));
    }
    for (int i = 0; i < interpolator.getX().size(); i++) {
        nodes.append(createNode(interpolator.getX()[i], interpolator.getY()[i]));
        mapSenderToControlPointIndex[nodes.back()] = i;
    }
    interpolationItem = new GraphicsInterpolationItem(&interpolator, 0.01, -1, 1, rect.width() * 0.5 / M_PI, -rect.height() * 0.5, this);
    interpolationItem->setPen(QPen(QBrush(Qt::black), 2));
    interpolationItem->setPos(0, 0.5 * (rect.top() + rect.bottom()));

    //interpolationIntegralItem = new GraphicsInterpolationItem(&interpolatorIntegral, 0.01, -1, 1, rect.width() * 0.5 / M_PI, -rect.height() * 0.5, this);
    interpolationIntegralItem = new GraphicsInterpolationItem(&interpolatorIntegral, 0.01, -1, 1, rect.width() * 0.5 / M_PI, -rect.height() * 0.5, this);
    interpolationIntegralItem->setPen(QPen(QBrush(Qt::black), 2, Qt::DotLine));
    interpolationIntegralItem->setPos(0, 0.5 * (rect.top() + rect.bottom()));

    // create the context menu:
    contextMenu.addAction(tr("Add a control point"), this, SLOT(increaseControlPoints()));
    contextMenu.addAction(tr("Delete the last control point"), this, SLOT(decreaseControlPoints()));
}

void LinearOscillatorGraphicsItem::increaseControlPoints()
{
    // send the control point increase event to the client and update our interpolators:
    interpolator = client->postIncreaseControlPoints();
    interpolationItem->updatePath();
    interpolatorIntegral = LinearIntegralInterpolator(interpolator);
    interpolationIntegralItem->updatePath();
    // update the existing nodes:
    for (int i = 0; i < nodes.size(); i++) {
        nodes[i]->setXScaled(interpolator.getX()[i]);
        nodes[i]->setYScaled(interpolator.getY()[i]);
    }
    // add a new node:
    nodes.append(createNode(interpolator.getX().back(), interpolator.getY().back()));
    mapSenderToControlPointIndex[nodes.back()] = nodes.size() - 1;
}

void LinearOscillatorGraphicsItem::decreaseControlPoints()
{
    if (interpolator.getX().size() > 2) {
        // remove the last node from our node map and delete it:
        mapSenderToControlPointIndex.remove(nodes.back());
        delete nodes.back();
        nodes.remove(nodes.size() - 1);
        // send the control point decrease event to the client and update our interpolators:
        interpolator = client->postDecreaseControlPoints();
        interpolationItem->updatePath();
        interpolatorIntegral = LinearIntegralInterpolator(interpolator);
        interpolationIntegralItem->updatePath();
        // update the remaining nodes:
        for (int i = 0; i < nodes.size(); i++) {
            nodes[i]->setXScaled(interpolator.getX()[i]);
            nodes[i]->setYScaled(interpolator.getY()[i]);
        }
    }
}

void LinearOscillatorGraphicsItem::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
    QGraphicsRectItem::mousePressEvent(event);
    if (!event->isAccepted() && (event->button() == Qt::RightButton)) {
        event->accept();
        // show a menu that allows removing and adding control points:
        contextMenu.exec(event->screenPos());
    }
}

void LinearOscillatorGraphicsItem::onNodePositionChangedScaled(QPointF position)
{
    // get the control point index:
    int index = mapSenderToControlPointIndex[sender()];
    if (index == 0) {
        position.setX(interpolator.getX()[0]);
    }
    if (index == interpolator.getX().size() - 1) {
        position.setX(interpolator.getX().back());
    }
    if ((index > 0) && (position.x() <= interpolator.getX()[index - 1])) {
        position.setX(interpolator.getX()[index - 1]);
    }
    if ((index < interpolator.getX().size() - 1) && (position.x() >= interpolator.getX()[index + 1])) {
        position.setX(interpolator.getX()[index + 1]);
    }
    LinearOscillatorParameters parameters;
    parameters.controlPoints = interpolator.getX().size();
    parameters.index = index;
    interpolator.getX()[index] = parameters.x = position.x();
    interpolator.getY()[index] = parameters.y = position.y();
    client->postEvent(parameters);
    interpolationItem->updatePath();
    interpolatorIntegral = LinearIntegralInterpolator(interpolator);
    interpolationIntegralItem->updatePath();
}

GraphicsNodeItem * LinearOscillatorGraphicsItem::createNode(qreal x, qreal y)
{
    GraphicsNodeItem *nodeItem = new GraphicsNodeItem(-5.0, -5.0, 10.0, 10.0, this);
    nodeItem->setPen(QPen(QBrush(qRgb(114, 159, 207)), 3));
    nodeItem->setBrush(QBrush(qRgb(52, 101, 164)));
    nodeItem->setZValue(1);
    nodeItem->setBounds(rect());
    nodeItem->setBoundsScaled(QRectF(QPointF(0, 1), QPointF(2 * M_PI, -1)));
    nodeItem->setXScaled(x);
    nodeItem->setYScaled(y);
    nodeItem->setSendPositionChanges(true);
    QObject::connect(nodeItem, SIGNAL(positionChangedScaled(QPointF)), this, SLOT(onNodePositionChangedScaled(QPointF)));
    return nodeItem;
}
