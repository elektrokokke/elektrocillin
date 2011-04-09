/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Elektrocillin is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Elektrocillin.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cubicsplinewaveshapingclient.h"
#include "graphicsnodeitem.h"
#include "graphicslineitem.h"
#include "cisi.h"
#include <QPen>

CubicSplineWaveShapingClient::CubicSplineWaveShapingClient(const QString &clientName, CubicSplineInterpolator *processWaveShaper_, CubicSplineInterpolator *guiWaveShaper_, size_t ringBufferSize) :
    EventProcessorClient(clientName, QStringList("Audio in"), QStringList("Audio out"), QStringList(), QStringList(), ringBufferSize),
    processWaveShaper(processWaveShaper_),
    guiWaveShaper(guiWaveShaper_)
{
}

CubicSplineWaveShapingClient::~CubicSplineWaveShapingClient()
{
    close();
    delete processWaveShaper;
    delete guiWaveShaper;
}

void CubicSplineWaveShapingClient::saveState(QDataStream &stream)
{
    EventProcessorClient::saveState(stream);
    guiWaveShaper->save(stream);
}

void CubicSplineWaveShapingClient::loadState(QDataStream &stream)
{
    EventProcessorClient::loadState(stream);
    guiWaveShaper->load(stream);
    processWaveShaper->changeControlPoints(guiWaveShaper->getX(), guiWaveShaper->getY());
}

QGraphicsItem * CubicSplineWaveShapingClient::createGraphicsItem()
{
    return new GraphicsInterpolatorEditItem(this, QRectF(0, 0, 420, 420), QRectF(-1, 1, 2, -2));
}

double CubicSplineWaveShapingClient::evaluate(double x, int *index)
{
    return guiWaveShaper->evaluate(x, index);
}

int CubicSplineWaveShapingClient::getNrOfControlPoints()
{
    return guiWaveShaper->getNrOfControlPoints();
}

QPointF CubicSplineWaveShapingClient::getControlPoint(int index)
{
    return guiWaveShaper->getControlPoint(index);
}

void CubicSplineWaveShapingClient::changeControlPoint(int index, double x, double y)
{
    guiWaveShaper->changeControlPoint(index, x, y);
    Interpolator::ChangeControlPointEvent *event = new Interpolator::ChangeControlPointEvent(index, x, y);
    postEvent(event);
}

void CubicSplineWaveShapingClient::addControlPoint(double x, double y)
{
    guiWaveShaper->addControlPoint(x, y);
    Interpolator::AddControlPointEvent *event = new Interpolator::AddControlPointEvent(x, y);
    postEvent(event);
}

void CubicSplineWaveShapingClient::deleteControlPoint(int index)
{
    guiWaveShaper->deleteControlPoint(index);
    Interpolator::DeleteControlPointEvent *event = new Interpolator::DeleteControlPointEvent(index);
    postEvent(event);
}

QString CubicSplineWaveShapingClient::getControlPointName(int index) const
{
    return guiWaveShaper->getControlPointName(index);
}

void CubicSplineWaveShapingClient::processAudio(const double *inputs, double *outputs, jack_nframes_t)
{
    outputs[0] = std::max(std::min(processWaveShaper->evaluate(inputs[0]), 1.0), -1.0);
}

bool CubicSplineWaveShapingClient::processEvent(const RingBufferEvent *event, jack_nframes_t)
{
    if (const Interpolator::InterpolatorEvent *event_ = dynamic_cast<const Interpolator::InterpolatorEvent*>(event)) {
        processWaveShaper->processInterpolatorEvent(event_);
        return true;
    } else {
        return false;
    }
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
        return "Shaper (cubic)";
    }
    JackClient * createClient(const QString &clientName)
    {
        QVector<double> xx, yy;
        xx.append(-1);
        yy.append(-1);
        xx.append(1);
        yy.append(1);
        return new CubicSplineWaveShapingClient(clientName, new CubicSplineInterpolator(xx, yy), new CubicSplineInterpolator(xx, yy));
    }
    static CubicSplineWaveShapingClientFactory factory;
};

CubicSplineWaveShapingClientFactory CubicSplineWaveShapingClientFactory::factory;

JackClientFactory * CubicSplineWaveShapingClient::getFactory()
{
    return &CubicSplineWaveShapingClientFactory::factory;
}

