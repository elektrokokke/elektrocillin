#ifndef LINEARMORPHOSCILLATORCLIENT_H
#define LINEARMORPHOSCILLATORCLIENT_H

/*
    Copyright 2011 Arne Jacobs

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

#include "oscillatorclient.h"
#include "linearmorphoscillator.h"
#include "graphicsinterpolatoredititem.h"

class LinearMorphOscillatorClient : public OscillatorClient
{
public:
    LinearMorphOscillatorClient(const QString &clientName, double frequencyModulationIntensity = 2.0/12.0, size_t ringBufferSize = 1024);
    virtual ~LinearMorphOscillatorClient();

    virtual JackClientFactory * getFactory();
    virtual void saveState(QDataStream &stream);
    virtual void loadState(QDataStream &stream);

    LinearMorphOscillator * getLinearMorphOscillator();
    LinearInterpolator * getState(int state);

    void postIncreaseControlPoints();
    void postDecreaseControlPoints();
    void postChangeControlPoint(int state, int index, double x, double y);

    QGraphicsItem * createGraphicsItem();
private:
    LinearInterpolator state[2];
};

class LinearMorphOscillatorGraphicsSubItem : public GraphicsInterpolatorEditItem
{
public:
    LinearMorphOscillatorGraphicsSubItem(const QRectF &rect, LinearMorphOscillatorClient *client, int state, QGraphicsItem *parent = 0);

    void setTwin(LinearMorphOscillatorGraphicsSubItem *twin);

protected:
    virtual void increaseControlPoints();
    virtual void decreaseControlPoints();
    virtual void changeControlPoint(int index, double x, double y);

private:
    LinearMorphOscillatorClient *client;
    int state;
    LinearMorphOscillatorGraphicsSubItem *twin;
};

class LinearMorphOscillatorGraphicsItem : public QGraphicsRectItem{
public:
    LinearMorphOscillatorGraphicsItem(const QRectF &rect, LinearMorphOscillatorClient *client, QGraphicsItem *parent = 0);
};


#endif // LINEARMORPHOSCILLATORCLIENT_H
