#include "cubicsplinewaveshapingclient.h"
#include "graphicsnodeitem.h"
#include "graphicslineitem.h"
#include "cisi.h"
#include <QPen>

CubicSplineWaveShapingClient::CubicSplineWaveShapingClient(const QString &clientName, size_t ringBufferSize) :
    EventProcessorClient<InterpolatorParameters>(clientName, QStringList("Audio in"), QStringList("Audio out"), ringBufferSize),
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

CubicSplineInterpolator * CubicSplineWaveShapingClient::getInterpolator()
{
    return &interpolator;
}

void CubicSplineWaveShapingClient::postIncreaseControlPoints()
{
    QVector<double> xx = interpolator.getX();
    QVector<double> yy = interpolator.getY();
    int size = xx.size() + 2;
    double stretchFactor = (double)(xx.size() - 1) / (double)(size - 1);
    for (int i = 0; i < xx.size() ; i++) {
        xx[i] = xx[i] * stretchFactor;
        yy[i] = yy[i] * stretchFactor;
    }
    xx.insert(0, -1);
    yy.insert(0, -1);
    xx.append(1);
    yy.append(1);
    QVector<InterpolatorParameters> parameterVector;
    for (int i = 0; i < xx.size(); i++) {
        InterpolatorParameters parameters;
        parameters.controlPoints = size;
        parameters.index = i;
        parameters.x = xx[i];
        parameters.y = yy[i];
        parameterVector.append(parameters);
    }
    postEvents(parameterVector);
    interpolator = CubicSplineInterpolator(xx, yy);
}

void CubicSplineWaveShapingClient::postDecreaseControlPoints()
{
    if (interpolator.getX().size() > 3) {
        QVector<double> xx = interpolator.getX();
        QVector<double> yy = interpolator.getY();
        int size = xx.size() - 2;
        xx.remove(0);
        yy.remove(0);
        xx.resize(size);
        yy.resize(size);
        double stretchFactor1 = 1.0 / -xx.first();
        double stretchFactor2 = 1.0 / xx.back();
        QVector<InterpolatorParameters> parameterVector;
        for (int i = 0; i < xx.size(); i++) {
            double stretchFactor = (i < size / 2 ? stretchFactor1 : stretchFactor2);
            xx[i] = xx[i] * stretchFactor;
            yy[i] = qMin(1.0, qMax(-1.0, yy[i] * stretchFactor));
            InterpolatorParameters parameters;
            parameters.controlPoints = size;
            parameters.index = i;
            parameters.x = xx[i];
            parameters.y = yy[i];
            parameterVector.append(parameters);
        }
        postEvents(parameterVector);
        interpolator = CubicSplineInterpolator(xx, yy);
    }
}

void CubicSplineWaveShapingClient::postChangeControlPoint(int index, int nrOfControlPoints, double x, double y)
{
    if (index == 0) {
       x = interpolator.getX()[0];
    }
    if (index == interpolator.getX().size() - 1) {
        x = interpolator.getX().back();
    }
    if ((index > 0) && (x <= interpolator.getX()[index - 1])) {
        return;
    }
    if ((index < interpolator.getX().size() - 1) && (x >= interpolator.getX()[index + 1])) {
        return;
    }
    QVector<double> xx = interpolator.getX();
    QVector<double> yy = interpolator.getY();
    InterpolatorParameters parameters;
    parameters.controlPoints = nrOfControlPoints;
    parameters.index = index;
    xx[index] = parameters.x = x;
    yy[index] = parameters.y = y;
    postEvent(parameters);
    interpolator = CubicSplineInterpolator(xx, yy);
}

QGraphicsItem * CubicSplineWaveShapingClient::createGraphicsItem(const QRectF &rect)
{
    return new CubicSplineWaveShapingGraphicsItem(this, rect);
}

void CubicSplineWaveShapingClient::processAudio(const double *inputs, double *outputs, jack_nframes_t)
{
    outputs[0] = std::max(std::min(interpolatorProcess.evaluate(inputs[0]), 1.0), -1.0);
}

void CubicSplineWaveShapingClient::processEvent(const InterpolatorParameters &event, jack_nframes_t)
{
    // set the interpolator parameters accordingly:
    QVector<double> xx = interpolator.getX();
    QVector<double> yy = interpolator.getY();
    xx.resize(event.controlPoints);
    yy.resize(event.controlPoints);
    xx[event.index] = event.x;
    yy[event.index] = event.y;
    interpolatorProcess = CubicSplineInterpolator(xx, yy);
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

void CubicSplineWaveShapingGraphicsItem::changeControlPoint(int index, int nrOfControlPoints, double x, double y) {
    client->postChangeControlPoint(index, nrOfControlPoints, x, y);
}

class CubicSplineWaveShapingClientFactory : public JackClientFactory
{
public:
    QString getName()
    {
        return "Cubic spline wave shaper";
    }
    JackClient * createClient(const QString &clientName)
    {
        return new CubicSplineWaveShapingClient(clientName);
    }
private:
    static CubicSplineWaveShapingClientFactory factory;
};

CubicSplineWaveShapingClientFactory CubicSplineWaveShapingClientFactory::factory;
