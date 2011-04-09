#ifndef INTEGRALOSCILLATORCLIENT_H
#define INTEGRALOSCILLATORCLIENT_H

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

#include "oscillatorclient.h"
#include "polynomialoscillator.h"
#include "graphicsinterpolatoredititem.h"
#include <QPen>

class PolynomialOscillatorClient : public OscillatorClient, public AbstractInterpolator
{
public:
    PolynomialOscillatorClient(const QString &clientName, PolynomialOscillator *processOscillator, PolynomialOscillator *guiOscillator, size_t ringBufferSize = 1024);
    virtual ~PolynomialOscillatorClient();

    virtual JackClientFactory * getFactory();
    virtual void saveState(QDataStream &stream);
    /**
      To call this method is only safe when the client is not running,
      as it accesses the internal oscillator object used by the Jack
      process thread in a non-threadsafe way.

      To change the oscillator parameters while the client is running use
      post...() methods.
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
private:
    PolynomialOscillator *processOscillator, *guiOscillator;
};

class IntegralOscillatorGraphicsItem : public GraphicsInterpolatorEditItem
{
public:
    IntegralOscillatorGraphicsItem(const QRectF &rect, PolynomialOscillatorClient *client, QGraphicsItem *parent = 0);

protected:
    virtual void increaseControlPoints();
    virtual void decreaseControlPoints();
    virtual void changeControlPoint(int index, double x, double y);

private:
    PolynomialOscillatorClient *client;
};

#endif // INTEGRALOSCILLATORCLIENT_H
