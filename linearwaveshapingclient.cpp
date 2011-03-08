#include "linearwaveshapingclient.h"
#include "graphicsnodeitem.h"
#include <QPen>
#include <QtGlobal>

LinearWaveShapingClient::LinearWaveShapingClient(const QString &clientName, size_t ringBufferSize) :
    EventProcessorClient(clientName, QStringList("Audio in"), QStringList("Audio out"), ringBufferSize)
{
    QVector<double> xx, yy;
    int nrOfControlPoints = 5;
    for (int i = 0; i < nrOfControlPoints; i++) {
        double value = (double)i / (double)(nrOfControlPoints - 1) * 2 - 1;
        xx.append(value);
        yy.append(value);
    }
    interpolator.changeControlPoints(xx, yy);
    interpolatorProcess.changeControlPoints(xx, yy);
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
    Interpolator::AddControlPointsEvent *event = new Interpolator::AddControlPointsEvent(true, true, true, true);
    interpolator.addControlPoints(event);
    postEvent(event);
}

void LinearWaveShapingClient::postDecreaseControlPoints()
{
    if (interpolator.getX().size() > 3) {
        Interpolator::DeleteControlPointsEvent *event = new Interpolator::DeleteControlPointsEvent(true, true, true, true);
        interpolator.deleteControlPoints(event);
        postEvent(event);
    }
}

void LinearWaveShapingClient::postChangeControlPoint(int index, double x, double y)
{
    Interpolator::ChangeControlPointEvent *event = new Interpolator::ChangeControlPointEvent(index, x, y);
    interpolator.changeControlPoint(event);
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

bool LinearWaveShapingClient::processEvent(const RingBufferEvent *event, jack_nframes_t)
{
    if (const Interpolator::ChangeControlPointEvent *event_ = dynamic_cast<const Interpolator::ChangeControlPointEvent*>(event)) {
        interpolatorProcess.changeControlPoint(event_);
        return true;
    } else if (const Interpolator::AddControlPointsEvent *event_ = dynamic_cast<const Interpolator::AddControlPointsEvent*>(event)) {
        interpolatorProcess.addControlPoints(event_);
        return true;
    } else if (const Interpolator::DeleteControlPointsEvent *event_ = dynamic_cast<const Interpolator::DeleteControlPointsEvent*>(event)) {
        interpolatorProcess.deleteControlPoints(event_);
        return true;
    }
    return false;
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
