#include "linearwaveshapingclient.h"
#include "graphicsnodeitem.h"
#include <QPen>

LinearWaveShapingClient::LinearWaveShapingClient(const QString &clientName, size_t ringBufferSize) :
    EventProcessorClient<LinearWaveShapingParameters>(clientName, QStringList("Audio in"), QStringList("Audio out"), ringBufferSize),
    interpolator(QVector<double>(5), QVector<double>(5))
{
    for (int i = 0; i < interpolator.getX().size(); i++) {
        interpolator.getX()[i] =  interpolator.getY()[i] = (double)i / (double)(interpolator.getX().size() - 1) * 2 - 1;
    }
    activateMidiInput(false);
}

LinearWaveShapingClient::~LinearWaveShapingClient()
{
    close();
}

const LinearInterpolator & LinearWaveShapingClient::getInterpolator() const
{
    return interpolator;
}

void LinearWaveShapingClient::processAudio(const double *inputs, double *outputs, jack_nframes_t)
{
    outputs[0] = interpolator.evaluate(inputs[0]);
}

void LinearWaveShapingClient::processEvent(const LinearWaveShapingParameters &event, jack_nframes_t)
{
    // set the interpolator's nr of control points:
    interpolator.getX().resize(event.controlPoints);
    interpolator.getY().resize(event.controlPoints);
    // set the interpolator control point at "index" accordingly:
    interpolator.getX()[event.index] = event.x;
    interpolator.getY()[event.index] = event.y;
}

LinearWaveShapingGraphicsItem::LinearWaveShapingGraphicsItem(const QRectF &rect, LinearWaveShapingClient *client_, QGraphicsItem *parent) :
    QGraphicsRectItem(rect, parent),
    client(client_),
    interpolator(client->getInterpolator()),
    interpolatorIntegral(interpolator)
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
        GraphicsNodeItem *nodeItem = new GraphicsNodeItem(-5.0, -5.0, 10.0, 10.0, this);
        nodeItem->setPen(QPen(QBrush(qRgb(114, 159, 207)), 3));
        nodeItem->setBrush(QBrush(qRgb(52, 101, 164)));
        nodeItem->setZValue(1);
        nodeItem->setBounds(rect);
        nodeItem->setBoundsScaled(QRectF(QPointF(-1, 1), QPointF(1, -1)));
        nodeItem->setXScaled(interpolator.getX()[i]);
        nodeItem->setYScaled(interpolator.getY()[i]);
        nodeItem->setSendPositionChanges(true);
        mapSenderToControlPointIndex[nodeItem] = i;
        QObject::connect(nodeItem, SIGNAL(positionChangedScaled(QPointF)), this, SLOT(onNodePositionChangedScaled(QPointF)));
    }
    interpolationItem = new GraphicsInterpolationItem(&interpolator, 0.01, -1, 1, rect.width() * 0.5, -rect.height() * 0.5, this);
    interpolationItem->setPen(QPen(QBrush(Qt::black), 2));
    interpolationItem->setPos(rect.center());

    interpolationIntegralItem = new GraphicsInterpolationItem(&interpolatorIntegral, 0.01, -1, 1, rect.width() * 0.5, -rect.height() * 0.5, this);
    interpolationIntegralItem->setPen(QPen(QBrush(Qt::black), 2, Qt::DotLine));
    interpolationIntegralItem->setPos(rect.center());
}

void LinearWaveShapingGraphicsItem::onNodePositionChangedScaled(QPointF position)
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
    LinearWaveShapingParameters parameters;
    parameters.controlPoints = interpolator.getX().size();
    parameters.index = index;
    interpolator.getX()[index] = parameters.x = position.x();
    interpolator.getY()[index] = parameters.y = position.y();
    client->postEvent(parameters);
    interpolationItem->updatePath();

    interpolatorIntegral = LinearIntegralInterpolator(interpolator);
    interpolationIntegralItem->updatePath();
}
