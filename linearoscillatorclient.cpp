#include "linearoscillatorclient.h"
#include "graphicsnodeitem.h"
#include <cmath>

LinearOscillatorClient::LinearOscillatorClient(const QString &clientName, size_t ringBufferSize) :
    EventProcessorClient<InterpolatorParameters>(clientName, new LinearOscillator(), ringBufferSize),
    interpolator(getLinearOscillator()->getLinearInterpolator()),
    interpolatorProcess(getLinearOscillator()->getLinearInterpolator())
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

LinearInterpolator * LinearOscillatorClient::getLinearInterpolator()
{
    return &interpolator;
}

void LinearOscillatorClient::postIncreaseControlPoints()
{
    int size = interpolator.getX().size() + 1;
    double stretchFactor = (double)(interpolator.getX().size() - 1) / (double)(size - 1);
    interpolator.getX().append(2 * M_PI);
    interpolator.getY().append(1);
    QVector<InterpolatorParameters> parameterVector;
    for (int i = 0; i < size; i++) {
        if (i < size - 1) {
            interpolator.getX()[i] = interpolator.getX()[i] * stretchFactor;
        }
        InterpolatorParameters parameters;
        parameters.controlPoints = size;
        parameters.index = i;
        parameters.x = interpolator.getX()[i];
        parameters.y = interpolator.getY()[i];
        parameterVector.append(parameters);
    }
    postEvents(parameterVector);
}

void LinearOscillatorClient::postDecreaseControlPoints()
{
    if (interpolator.getX().size() > 2) {
        int size = interpolator.getX().size() - 1;
        interpolator.getX().resize(size);
        interpolator.getY().resize(size);
        double stretchFactor = 2 * M_PI / interpolator.getX().back();
        QVector<InterpolatorParameters> parameterVector;
        for (int i = 0; i < size; i++) {
            interpolator.getX()[i] = interpolator.getX()[i] * stretchFactor;
            InterpolatorParameters parameters;
            parameters.controlPoints = size;
            parameters.index = i;
            parameters.x = interpolator.getX()[i];
            parameters.y = interpolator.getY()[i];
            parameterVector.append(parameters);
        }
        postEvents(parameterVector);
    }
}

void LinearOscillatorClient::postChangeControlPoint(int index, int nrOfControlPoints, double x, double y)
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
    InterpolatorParameters parameters;
    parameters.controlPoints = nrOfControlPoints;
    parameters.index = index;
    interpolator.getX()[index] = parameters.x = x;
    interpolator.getY()[index] = parameters.y = y;
    postEvent(parameters);
}

void LinearOscillatorClient::processEvent(const InterpolatorParameters &event, jack_nframes_t)
{
    // set the interpolator's nr of control points:
    interpolatorProcess.getX().resize(event.controlPoints);
    interpolatorProcess.getY().resize(event.controlPoints);
    // set the interpolator control point at "index" accordingly:
    interpolatorProcess.getX()[event.index] = event.x;
    interpolatorProcess.getY()[event.index] = event.y;
    // update the integral:
    getLinearOscillator()->setLinearInterpolator(interpolatorProcess);
}

LinearOscillatorGraphicsItem::LinearOscillatorGraphicsItem(const QRectF &rect, LinearOscillatorClient *client_, QGraphicsItem *parent) :
    GraphicsInterpolatorEditItem(client_->getLinearInterpolator(), rect, QRectF(0, 1, 2.0 * M_PI, -2), parent),
    client(client_)
{}

void LinearOscillatorGraphicsItem::increaseControlPoints()
{
    client->postIncreaseControlPoints();
}

void LinearOscillatorGraphicsItem::decreaseControlPoints()
{
    client->postDecreaseControlPoints();
}

void LinearOscillatorGraphicsItem::changeControlPoint(int index, int nrOfControlPoints, double x, double y)
{
    client->postChangeControlPoint(index, nrOfControlPoints, x, y);
}
