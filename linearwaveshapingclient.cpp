#include "linearwaveshapingclient.h"
#include "graphicsnodeitem.h"
#include <QPen>

const int LinearWaveShapingClient::controlPointCount = sizeof(LinearWaveShapingParameters::x) / sizeof(double);

LinearWaveShapingClient::LinearWaveShapingClient(const QString &clientName, size_t ringBufferSize) :
    EventProcessorClient<LinearWaveShapingParameters>(clientName, QStringList("audio_in"), QStringList("shaped_out"), ringBufferSize),
    interpolator(QVector<double>(controlPointCount), QVector<double>(controlPointCount))
{
    for (int i = 0; i < controlPointCount; i++) {
        parameters.x[i] = parameters.y[i] = interpolator.getX()[i] =  interpolator.getY()[i] = (double)i / (double)(controlPointCount - 1);
    }
    deactivateMidiInput();
}

LinearWaveShapingClient::~LinearWaveShapingClient()
{
    close();
}

const LinearInterpolator & LinearWaveShapingClient::getInterpolator() const
{
    return interpolator;
}

const LinearWaveShapingParameters & LinearWaveShapingClient::getParameters() const
{
    return parameters;
}

void LinearWaveShapingClient::processAudio(const double *inputs, double *outputs, jack_nframes_t)
{
    if (inputs[0] >= 0) {
        outputs[0] = interpolator.evaluate(inputs[0]);
    } else {
        outputs[0] = -interpolator.evaluate(-inputs[0]);
    }
}

void LinearWaveShapingClient::processEvent(const LinearWaveShapingParameters &event, jack_nframes_t)
{
    parameters = event;
    // set the interpolator parameters accordingly:
    for (int i = 0; i < controlPointCount; i++) {
        interpolator.getX()[i] = event.x[i];
        interpolator.getY()[i] = event.y[i];
    }
}

LinearWaveShapingGraphicsItem::LinearWaveShapingGraphicsItem(const QRectF &rect, LinearWaveShapingClient *client_, QGraphicsItem *parent) :
    QGraphicsRectItem(rect, parent),
    client(client_),
    parameters(client->getParameters()),
    interpolator(client->getInterpolator()),
    interpolatorIntegral(&interpolator)
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
    for (int i = 0; i < LinearWaveShapingClient::controlPointCount; i++) {
        GraphicsNodeItem *nodeItem = new GraphicsNodeItem(-5.0, -5.0, 10.0, 10.0, this);
        nodeItem->setPen(QPen(QBrush(qRgb(114, 159, 207)), 3));
        nodeItem->setBrush(QBrush(qRgb(52, 101, 164)));
        nodeItem->setZValue(1);
        nodeItem->setBounds(rect);
        nodeItem->setBoundsScaled(QRectF(QPointF(0, 1), QPointF(1, 0)));
        nodeItem->setPos(QPointF(rect.left() + rect.width() * parameters.x[i], rect.bottom() - rect.height() * parameters.y[i]));
        nodeItem->setSendPositionChanges(true);
        mapSenderToControlPointIndex[nodeItem] = i;
        QObject::connect(nodeItem, SIGNAL(positionChangedScaled(QPointF)), this, SLOT(onNodePositionChangedScaled(QPointF)));
    }
    interpolationItem = new GraphicsInterpolationItem(&interpolator, 0.01, 0, 1, rect.width(), -rect.height(), this);
    interpolationItem->setPen(QPen(QBrush(Qt::black), 2));
    interpolationItem->setPos(rect.bottomLeft());

    interpolationIntegralItem = new GraphicsInterpolationItem(&interpolatorIntegral, 0.01, 0, 1, rect.width(), -rect.height(), this);
    interpolationIntegralItem->setPen(QPen(QBrush(Qt::black), 2, Qt::DotLine));
    interpolationIntegralItem->setPos(rect.bottomLeft());
}

void LinearWaveShapingGraphicsItem::onNodePositionChangedScaled(QPointF position)
{
    // get the control point index:
    int index = mapSenderToControlPointIndex[sender()];
    if (index == 0) {
        position.setX(0);
    }
    if (index == LinearWaveShapingClient::controlPointCount - 1) {
        position.setX(1);
    }
    if ((index > 0) && (position.x() <= parameters.x[index - 1])) {
        return;
    }
    if ((index < LinearWaveShapingClient::controlPointCount - 1) && (position.x() >= parameters.x[index + 1])) {
        return;
    }
    interpolator.getX()[index] = parameters.x[index] = position.x();
    interpolator.getY()[index] = parameters.y[index] = position.y();
    client->postEvent(parameters);
    interpolationItem->updatePath();

    interpolatorIntegral = LinearIntegralInterpolator(&interpolator);
    interpolationIntegralItem->updatePath();
}
