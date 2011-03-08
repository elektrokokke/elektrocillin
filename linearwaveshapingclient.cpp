#include "linearwaveshapingclient.h"
#include "graphicsnodeitem.h"
#include <QPen>
#include <QtGlobal>

LinearWaveShapingClient::LinearWaveShapingClient(const QString &clientName, size_t ringBufferSize) :
    EventProcessorClient(clientName, QStringList("Audio in"), QStringList("Audio out"), ringBufferSize)
{
    Interpolator::ChangeAllControlPointsEvent event;
    int nrOfControlPoints = 5;
    for (int i = 0; i < nrOfControlPoints; i++) {
        double value = (double)i / (double)(nrOfControlPoints - 1) * 2 - 1;
        event.xx.append(value);
        event.yy.append(value);
    }
    interpolator.processEvent(&event);
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
    Interpolator::ChangeAllControlPointsEvent *event = new Interpolator::ChangeAllControlPointsEvent(interpolator);
    int size = event->xx.size() + 2;
    double stretchFactor = (double)(event->xx.size() - 1) / (double)(size - 1);
    event->xx.insert(0, event->xx.first());
    event->yy.insert(0, event->yy.first());
    event->xx.append(event->xx.last());
    event->yy.append(event->yy.last());
    for (int i = 1; i < size - 1; i++) {
        event->xx[i] *= stretchFactor;
        event->yy[i] *= stretchFactor;
    }
    interpolator.processEvent(event);
    postEvent(event);
}

void LinearWaveShapingClient::postDecreaseControlPoints()
{
    if (interpolator.getX().size() > 3) {
        Interpolator::ChangeAllControlPointsEvent *event = new Interpolator::ChangeAllControlPointsEvent(interpolator);
        int size = event->xx.size() - 2;
        event->xx.remove(0);
        event->yy.remove(0);
        event->xx.resize(size);
        event->yy.resize(size);
        double stretchFactor1 = 1.0 / -event->xx.first();
        double stretchFactor2 = 1.0 / event->xx.back();
        for (int i = 0; i < size; i++) {
            double stretchFactor = (i < size / 2 ? stretchFactor1 : stretchFactor2);
            event->xx[i] *= stretchFactor;
            event->yy[i] = qMin(1.0, qMax(-1.0, event->yy[i] * stretchFactor));
        }
        interpolator.processEvent(event);
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

QGraphicsItem * LinearWaveShapingClient::createGraphicsItem()
{
    return new LinearWaveShapingGraphicsItem(QRectF(0, 0, 600, 420), this);
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
