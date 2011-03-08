#include "cubicsplinewaveshapingclient.h"
#include "graphicsnodeitem.h"
#include "graphicslineitem.h"
#include "cisi.h"
#include <QPen>

CubicSplineWaveShapingClient::CubicSplineWaveShapingClient(const QString &clientName, size_t ringBufferSize) :
    EventProcessorClient(clientName, QStringList("Audio in"), QStringList("Audio out"), ringBufferSize),
    interpolator(QVector<double>(), QVector<double>(), QVector<double>()),
    interpolatorProcess(QVector<double>(), QVector<double>(), QVector<double>())
{
    QVector<double> xx, yy;
    for (double t = -1.0; t <= 1.0; t += 0.125) {
        xx.append(t);
        yy.append(Cisi::si(t * 4.0 * M_PI) / M_PI);
    }
//    QVector<double> xx, yy;
//    xx.append(-1);
//    yy.append(-1);
//    xx.append(0);
//    yy.append(0);
//    xx.append(1);
//    yy.append(1);
    interpolator = interpolatorProcess = CubicSplineInterpolator(xx, yy);
    activateMidiInput(false);
}

CubicSplineWaveShapingClient::~CubicSplineWaveShapingClient()
{
    close();
}

void CubicSplineWaveShapingClient::saveState(QDataStream &stream)
{
    interpolator.save(stream);
}

void CubicSplineWaveShapingClient::loadState(QDataStream &stream)
{
    interpolator.load(stream);
    interpolatorProcess = interpolator;
}

CubicSplineInterpolator * CubicSplineWaveShapingClient::getInterpolator()
{
    return &interpolator;
}

void CubicSplineWaveShapingClient::postIncreaseControlPoints()
{
    Interpolator::AddControlPointsEvent *event = new Interpolator::AddControlPointsEvent(true, true, true, true);
    interpolator.addControlPoints(event);
    postEvent(event);
}

void CubicSplineWaveShapingClient::postDecreaseControlPoints()
{
    if (interpolator.getX().size() > 3) {
        Interpolator::DeleteControlPointsEvent *event = new Interpolator::DeleteControlPointsEvent(true, true, true, true);
        interpolator.deleteControlPoints(event);
        postEvent(event);
    }
}

void CubicSplineWaveShapingClient::postChangeControlPoint(int index, double x, double y)
{
    Interpolator::ChangeControlPointEvent *event = new Interpolator::ChangeControlPointEvent(index, x, y);
    interpolator.changeControlPoint(event);
    postEvent(event);
}

QGraphicsItem * CubicSplineWaveShapingClient::createGraphicsItem()
{
    return new CubicSplineWaveShapingGraphicsItem(this, QRectF(0, 0, 600, 420));
}

void CubicSplineWaveShapingClient::processAudio(const double *inputs, double *outputs, jack_nframes_t)
{
    outputs[0] = std::max(std::min(interpolatorProcess.evaluate(inputs[0]), 1.0), -1.0);
}

bool CubicSplineWaveShapingClient::processEvent(const RingBufferEvent *event, jack_nframes_t)
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

CubicSplineWaveShapingGraphicsItem::CubicSplineWaveShapingGraphicsItem(CubicSplineWaveShapingClient *client_, const QRectF &rect, QGraphicsItem *parent) :
    GraphicsInterpolatorEditItem(client_->getInterpolator(), rect, QRectF(-1, 1, 2, -2), parent),
    client(client_)
{
}

void CubicSplineWaveShapingGraphicsItem::increaseControlPoints() {
    return client->postIncreaseControlPoints();
}

void CubicSplineWaveShapingGraphicsItem::decreaseControlPoints() {
    return client->postDecreaseControlPoints();
}

void CubicSplineWaveShapingGraphicsItem::changeControlPoint(int index, double x, double y) {
    client->postChangeControlPoint(index, x, y);
}

class CubicSplineWaveShapingClientFactory : public JackClientFactory
{
public:
    CubicSplineWaveShapingClientFactory()
    {
        JackClientSerializer::getInstance()->registerFactory(this);
    }
    QString getName()
    {
        return "Cubic spline wave shaper";
    }
    JackClient * createClient(const QString &clientName)
    {
        return new CubicSplineWaveShapingClient(clientName);
    }
    static CubicSplineWaveShapingClientFactory factory;
};

CubicSplineWaveShapingClientFactory CubicSplineWaveShapingClientFactory::factory;

JackClientFactory * CubicSplineWaveShapingClient::getFactory()
{
    return &CubicSplineWaveShapingClientFactory::factory;
}

