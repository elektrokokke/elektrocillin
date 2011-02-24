#include "linearoscillatorclient.h"
#include "graphicsnodeitem.h"
#include <cmath>

LinearOscillatorClient::LinearOscillatorClient(const QString &clientName, size_t ringBufferSize) :
    OscillatorClient(clientName, new LinearOscillator(), ringBufferSize),
    interpolator(getLinearOscillator()->getLinearInterpolator())
{
}

LinearOscillatorClient::~LinearOscillatorClient()
{
    close();
}

LinearOscillator * LinearOscillatorClient::getLinearOscillator()
{
    return (LinearOscillator*)getAudioProcessor();
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
    for (int i = size - 1; i >= 0; i--) {
        if (i < size - 1) {
            interpolator.getX()[i] = interpolator.getX()[i] * stretchFactor;
        }
    }
    LinearOscillator::ChangeAllControlPointsEvent *event = new LinearOscillator::ChangeAllControlPointsEvent();
    event->xx = interpolator.getX();
    event->yy = interpolator.getY();
    postEvent(event);
}

void LinearOscillatorClient::postDecreaseControlPoints()
{
    if (interpolator.getX().size() > 2) {
        int size = interpolator.getX().size() - 1;
        interpolator.getX().resize(size);
        interpolator.getY().resize(size);
        double stretchFactor = 2 * M_PI / interpolator.getX().back();
        for (int i = size - 1; i >= 0; i--) {
            interpolator.getX()[i] = interpolator.getX()[i] * stretchFactor;
        }
        LinearOscillator::ChangeAllControlPointsEvent *event = new LinearOscillator::ChangeAllControlPointsEvent();
        event->xx = interpolator.getX();
        event->yy = interpolator.getY();
        postEvent(event);
    }
}

void LinearOscillatorClient::postChangeControlPoint(int index, double x, double y)
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
    LinearOscillator::ChangeControlPointEvent *event = new LinearOscillator::ChangeControlPointEvent();
    event->index = index;
    interpolator.getX()[index] = event->x = x;
    interpolator.getY()[index] = event->y = y;
    postEvent(event);
}

QGraphicsItem * LinearOscillatorClient::createGraphicsItem(const QRectF &rect)
{
    QGraphicsRectItem *rectItem = new QGraphicsRectItem(rect);
    rectItem->setPen(QPen(Qt::NoPen));
    QRectF rectGain(rect.x(), rect.y(), 16, rect.height());
    QRectF rectOscillator = rect.adjusted(rectGain.width(), 0, 0, 0);
    (new OscillatorClientGraphicsItem(rectGain, this))->setParentItem(rectItem);
    (new LinearOscillatorGraphicsItem(rectOscillator, this))->setParentItem(rectItem);
    return rectItem;
}

void LinearOscillatorClient::processEvent(const RingBufferEvent *event, jack_nframes_t time)
{
    if (const LinearOscillator::ChangeControlPointEvent *changeControlPointEvent = dynamic_cast<const LinearOscillator::ChangeControlPointEvent*>(event)) {
        getLinearOscillator()->processEvent(changeControlPointEvent, time);
    } else if (const LinearOscillator::ChangeAllControlPointsEvent *changeAllControlPointsEvent = dynamic_cast<const LinearOscillator::ChangeAllControlPointsEvent*>(event)) {
        getLinearOscillator()->processEvent(changeAllControlPointsEvent, time);
    } else {
        OscillatorClient::processEvent(event, time);
    }
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
    client->postChangeControlPoint(index, x, y);
}
