#ifndef SPLINEWAVESHAPINGCLIENT_H
#define SPLINEWAVESHAPINGCLIENT_H

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

#include "eventprocessorclient.h"
#include "cubicsplineinterpolator.h"
#include "graphicsinterpolatoredititem.h"

class CubicSplineWaveShapingClient : public EventProcessorClient, public AbstractInterpolator
{
public:
    CubicSplineWaveShapingClient(const QString &clientName, CubicSplineInterpolator *processWaveShaper, CubicSplineInterpolator *guiWaveShaper, size_t ringBufferSize = 1024);
    virtual ~CubicSplineWaveShapingClient();

    virtual JackClientFactory * getFactory();
    virtual void saveState(QDataStream &stream);
    virtual void loadState(QDataStream &stream);
    QGraphicsItem * createGraphicsItem();

    // Implemented from AbstractInterpolator:
    virtual double evaluate(double x, int *index = 0);
    virtual int getNrOfControlPoints();
    virtual QPointF getControlPoint(int index);
    virtual void changeControlPoint(int index, double x, double y);
    virtual void addControlPoint(double x, double y);
    virtual void deleteControlPoint(int index);
    virtual QString getControlPointName(int index) const;
protected:
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);
    virtual bool processEvent(const RingBufferEvent *event, jack_nframes_t time);

private:
    CubicSplineInterpolator *processWaveShaper, *guiWaveShaper;
};

#endif // SPLINEWAVESHAPINGCLIENT_H
