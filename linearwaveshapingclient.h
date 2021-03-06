#ifndef LINEARWAVESHAPINGCLIENT_H
#define LINEARWAVESHAPINGCLIENT_H

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

#include "linearinterpolator.h"
#include "parameterclient.h"
#include "graphicsinterpolatoredititem.h"

class LinearWaveShaper : public AudioProcessor, public EventProcessor, public ParameterProcessor, public LinearInterpolator
{
public:
    LinearWaveShaper();
    // reimplemented from Interpolator; change the behaviour when adding/changing control points:
    virtual void addControlPoint(double x, double y);
    virtual void changeControlPoint(int index, double x, double y);
    // reimplemented from AudioProcessor:
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);
    // reimplemented from EventProcessor:
    virtual bool processEvent(const RingBufferEvent *event, jack_nframes_t time);
};

class LinearWaveShapingClient : public ParameterClient, public AbstractInterpolator
{
    Q_OBJECT
public:
    LinearWaveShapingClient(const QString &clientName, LinearWaveShaper *processWaveShaper, LinearWaveShaper * guiWaveShaper, size_t ringBufferSize = 1024);
    virtual ~LinearWaveShapingClient();

    virtual JackClientFactory * getFactory();
    virtual void saveState(QDataStream &stream);
    /**
      To call this method is only safe when the client is not running,
      as it accesses the internal interpolator object used by the Jack
      process thread in a non-threadsafe way.

      To change the state while the client is running use
      postEvent() with a InterpolatorProcessor::InterpolatorEvent object
      or any of the other post...() methods.
      */
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
signals:
    void changedXSteps(int steps);
    void changedYSteps(int steps);
protected:
    // Reimplemented from ParameterClient:
    virtual void onChangedParameterValue(int index, double value, double min, double max);
private:
    LinearWaveShaper *processWaveShaper, *guiWaveShaper;
};

#endif // LINEARWAVESHAPINGCLIENT_H
