#include "linearwaveshapingclient.h"
#include "graphicsnodeitem.h"
#include <QPen>
#include <QtGlobal>

LinearWaveShapingClient::LinearWaveShapingClient(const QString &clientName, size_t ringBufferSize) :
    EventProcessorClient(clientName, QStringList("Audio in"), QStringList("Audio out"), ringBufferSize),
    interpolator(QVector<double>(5), QVector<double>(5)),
    interpolatorProcess(QVector<double>(5), QVector<double>(5))
{
    for (int i = 0; i < interpolator.getX().size(); i++) {
        interpolator.getX()[i] =  interpolator.getY()[i] = interpolatorProcess.getX()[i] = interpolatorProcess.getY()[i] = (double)i / (double)(interpolator.getX().size() - 1) * 2 - 1;
    }
    activateMidiInput(false);
}

LinearWaveShapingClient::~LinearWaveShapingClient()
{
    close();
}

void LinearWaveShapingClient::saveState(QDataStream &stream)
{
    interpolator.save(stream);
}

void LinearWaveShapingClient::loadState(QDataStream &stream)
{
    interpolator.load(stream);
    interpolatorProcess = interpolator;
}

LinearInterpolator * LinearWaveShapingClient::getLinearInterpolator()
{
    return &interpolator;
}

void LinearWaveShapingClient::postIncreaseControlPoints()
{
    int size = interpolator.getX().size() + 2;
    double stretchFactor = (double)(interpolator.getX().size() - 1) / (double)(size - 1);
    for (int i = 0; i < interpolator.getX().size() ; i++) {
        interpolator.getX()[i] = interpolator.getX()[i] * stretchFactor;
        interpolator.getY()[i] = interpolator.getY()[i] * stretchFactor;
    }
    interpolator.getX().insert(0, -1);
    interpolator.getY().insert(0, -1);
    interpolator.getX().append(1);
    interpolator.getY().append(1);
    Interpolator::ChangeAllControlPointsEvent *event = new Interpolator::ChangeAllControlPointsEvent();
    event->xx = interpolator.getX();
    event->yy = interpolator.getY();
    postEvent(event);
}

void LinearWaveShapingClient::postDecreaseControlPoints()
{
    if (interpolator.getX().size() > 3) {
        int size = interpolator.getX().size() - 2;
        interpolator.getX().remove(0);
        interpolator.getY().remove(0);
        interpolator.getX().resize(size);
        interpolator.getY().resize(size);
        double stretchFactor1 = 1.0 / -interpolator.getX().first();
        double stretchFactor2 = 1.0 / interpolator.getX().back();
        for (int i = 0; i < interpolator.getX().size(); i++) {
            double stretchFactor = (i < size / 2 ? stretchFactor1 : stretchFactor2);
            interpolator.getX()[i] = interpolator.getX()[i] * stretchFactor;
            interpolator.getY()[i] = qMin(1.0, qMax(-1.0, interpolator.getY()[i] * stretchFactor));
        }
        Interpolator::ChangeAllControlPointsEvent *event = new Interpolator::ChangeAllControlPointsEvent();
        event->xx = interpolator.getX();
        event->yy = interpolator.getY();
        postEvent(event);
    }
}

void LinearWaveShapingClient::postChangeControlPoint(int index, double x, double y)
{
    if (index == 0) {
       x = interpolator.getX()[0];
    }
    if (index == interpolator.getX().size() - 1) {
        x = interpolator.getX().back();
    }
    if ((index > 0) && (x <= interpolator.getX()[index - 1])) {
        x = interpolator.getX()[index - 1];
    }
    if ((index < interpolator.getX().size() - 1) && (x >= interpolator.getX()[index + 1])) {
        x = interpolator.getX()[index + 1];
    }
    Interpolator::ChangeControlPointEvent *event = new Interpolator::ChangeControlPointEvent();
    event->index = index;
    event->x = x;
    event->y = y;
    interpolator.processEvent(event);
    postEvent(event);
}

QGraphicsItem * LinearWaveShapingClient::createGraphicsItem(const QRectF &rect)
{
    return new LinearWaveShapingGraphicsItem(rect, this);
}

void LinearWaveShapingClient::processAudio(const double *inputs, double *outputs, jack_nframes_t)
{
    outputs[0] = interpolatorProcess.evaluate(inputs[0]);
}

void LinearWaveShapingClient::processEvent(const RingBufferEvent *event, jack_nframes_t)
{
    if (const Interpolator::ChangeControlPointEvent *changeControlPointEvent = dynamic_cast<const Interpolator::ChangeControlPointEvent*>(event)) {
        interpolatorProcess.processEvent(changeControlPointEvent);
    } else if (const Interpolator::ChangeAllControlPointsEvent *changeAllControlPointsEvent = dynamic_cast<const Interpolator::ChangeAllControlPointsEvent*>(event)) {
        interpolatorProcess.processEvent(changeAllControlPointsEvent);
    }
}

LinearWaveShapingGraphicsItem::LinearWaveShapingGraphicsItem(const QRectF &rect, LinearWaveShapingClient *client_, QGraphicsItem *parent) :
    GraphicsInterpolatorEditItem(client_->getLinearInterpolator(), rect, QRectF(-1, 1, 2, -2), parent),
    client(client_)
{
}

void LinearWaveShapingGraphicsItem::increaseControlPoints() {
    return client->postIncreaseControlPoints();
}

void LinearWaveShapingGraphicsItem::decreaseControlPoints() {
    return client->postDecreaseControlPoints();
}

void LinearWaveShapingGraphicsItem::changeControlPoint(int index, double x, double y) {
    client->postChangeControlPoint(index, x, y);
}

class LinearWaveShapingClientFactory : public JackClientFactory
{
public:
    LinearWaveShapingClientFactory()
    {
        JackClientSerializer::getInstance()->registerFactory(this);
    }
    QString getName()
    {
        return "Piecewise linear wave shaper";
    }
    JackClient * createClient(const QString &clientName)
    {
        return new LinearWaveShapingClient(clientName);
    }
    static LinearWaveShapingClientFactory factory;
};

LinearWaveShapingClientFactory LinearWaveShapingClientFactory::factory;

JackClientFactory * LinearWaveShapingClient::getFactory()
{
    return &LinearWaveShapingClientFactory::factory;
}
