/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "linearwaveshapingclient.h"
#include "graphicsnodeitem.h"
#include <QPen>
#include <QtGlobal>

LinearWaveShaper::LinearWaveShaper() :
    AudioProcessor(QStringList("Audio in"), QStringList("Audio out"))
{
    QVector<double> xx, yy;
    xx.append(-1);
    yy.append(-1);
    xx.append(1);
    yy.append(1);
    changeControlPoints(xx, yy);
    // register numeric parameters:
    registerParameter("X steps", 0, 0, 32, 1);
    registerParameter("Y steps", 0, 0, 12, 1);
}

void LinearWaveShaper::changeControlPoint(int index, double x, double y)
{
    double xSteps = getParameter(0).value;
    double ySteps = getParameter(1).value;
    if (xSteps) {
        // discretize the X coordinate:
        x = qRound(x * xSteps) / xSteps;
    }
    if (ySteps) {
        // discretize the Y coordinate:
        y = qRound(y * ySteps) / ySteps;
    }
    LinearInterpolator::changeControlPoint(index, x, y);
}

void LinearWaveShaper::processAudio(const double *inputs, double *outputs, jack_nframes_t)
{
    outputs[0] = evaluate(inputs[0]);
}

bool LinearWaveShaper::processEvent(const RingBufferEvent *event, jack_nframes_t)
{
    if (const Interpolator::InterpolatorEvent *event_ = dynamic_cast<const Interpolator::InterpolatorEvent*>(event)) {
        processInterpolatorEvent(event_);
        return true;
    } else {
        return false;
    }
}

LinearWaveShapingClient::LinearWaveShapingClient(const QString &clientName, LinearWaveShaper *processWaveShaper_, LinearWaveShaper * guiWaveShaper_, size_t ringBufferSize) :
    ParameterClient(clientName, processWaveShaper_, 0, processWaveShaper_, processWaveShaper_, guiWaveShaper_, ringBufferSize),
    processWaveShaper(processWaveShaper_),
    guiWaveShaper(guiWaveShaper_)
{
}

LinearWaveShapingClient::~LinearWaveShapingClient()
{
    close();
    delete processWaveShaper;
    delete guiWaveShaper;
}

void LinearWaveShapingClient::saveState(QDataStream &stream)
{
    EventProcessorClient::saveState(stream);
    guiWaveShaper->save(stream);
}

void LinearWaveShapingClient::loadState(QDataStream &stream)
{
    EventProcessorClient::loadState(stream);
    guiWaveShaper->load(stream);
    processWaveShaper->changeControlPoints(guiWaveShaper->getX(), guiWaveShaper->getY());
}

QGraphicsItem * LinearWaveShapingClient::createGraphicsItem()
{
    int padding = 4;
    QGraphicsRectItem *item = new QGraphicsRectItem();
    QGraphicsItem *parameterItem = ParameterClient::createGraphicsItem();
    QRectF rect = QRect(0, 0, 420, 420);
    rect = rect.translated(parameterItem->boundingRect().width() + 2 * padding, padding);
    parameterItem->setPos(padding, padding);
    parameterItem->setParentItem(item);
    new GraphicsInterpolatorEditItem(this, rect, QRectF(-1, 1, 2, -2), item);
    item->setRect((rect | parameterItem->boundingRect().translated(parameterItem->pos())).adjusted(-padding, -padding, padding, padding));
    item->setPen(QPen(QBrush(Qt::black), 1));
    item->setBrush(QBrush(Qt::white));
    return item;
}

double LinearWaveShapingClient::evaluate(double x, int *index)
{
    return guiWaveShaper->evaluate(x, index);
}

int LinearWaveShapingClient::getNrOfControlPoints()
{
    return guiWaveShaper->getNrOfControlPoints();
}

QPointF LinearWaveShapingClient::getControlPoint(int index)
{
    return guiWaveShaper->getControlPoint(index);
}

void LinearWaveShapingClient::changeControlPoint(int index, double x, double y)
{
    guiWaveShaper->changeControlPoint(index, x, y);
    Interpolator::ChangeControlPointEvent *event = new Interpolator::ChangeControlPointEvent(index, x, y);
    postEvent(event);
}

void LinearWaveShapingClient::addControlPoint(double x, double y)
{
    guiWaveShaper->addControlPoint(x, y);
    Interpolator::AddControlPointEvent *event = new Interpolator::AddControlPointEvent(x, y);
    postEvent(event);
}

void LinearWaveShapingClient::deleteControlPoint(int index)
{
    guiWaveShaper->deleteControlPoint(index);
    Interpolator::DeleteControlPointEvent *event = new Interpolator::DeleteControlPointEvent(index);
    postEvent(event);
}

QString LinearWaveShapingClient::getControlPointName(int index) const
{
    return guiWaveShaper->getControlPointName(index);
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
        return new LinearWaveShapingClient(clientName, new LinearWaveShaper(), new LinearWaveShaper());
    }
    static LinearWaveShapingClientFactory factory;
};

LinearWaveShapingClientFactory LinearWaveShapingClientFactory::factory;

JackClientFactory * LinearWaveShapingClient::getFactory()
{
    return &LinearWaveShapingClientFactory::factory;
}
