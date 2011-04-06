#ifndef ENVELOPECLIENT_H
#define ENVELOPECLIENT_H

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

#include "envelope.h"
#include "parameterclient.h"
#include "graphicsinterpolatoredititem.h"
#include "graphicsdiscretecontrolitem.h"
#include <QGraphicsRectItem>
#include <QPen>
#include <QBrush>
#include <QFont>

class EnvelopeClient : public ParameterClient, public AbstractInterpolator
{
public:
    /**
      Creates a new envelope client with the given name.

      This object takes ownership of the given envelope object, i.e., it
      is deleted at destruction time.
      */
    EnvelopeClient(const QString &clientName, Envelope *processEnvelope, Envelope *guiEnvelope, size_t ringBufferSize = (2 << 16));
    virtual ~EnvelopeClient();

    virtual JackClientFactory * getFactory();
    virtual void saveState(QDataStream &stream);
    virtual void loadState(QDataStream &stream);
    QGraphicsItem * createGraphicsItem();

    void postChangeSustainIndex(int sustainIndex);
    // Implemented from AbstractInterpolator:
    virtual double evaluate(double x, int *index = 0);
    virtual int getNrOfControlPoints();
    virtual QPointF getControlPoint(int index);
    virtual void changeControlPoint(int index, double x, double y);
    virtual void addControlPoint(double x, double y);
    virtual void deleteControlPoint(int index);
    virtual QString getControlPointName(int index) const;
private:
    Envelope *processEnvelope, *guiEnvelope;
};

class EnvelopeGraphicsItem : public QObject, public GraphicsInterpolatorEditItem
{
    Q_OBJECT
public:
    EnvelopeGraphicsItem(const QRectF &rect, EnvelopeClient *client, QGraphicsItem *parent = 0);
private slots:
    void updateInterpolator();
private:
    EnvelopeClient *client;
};

#endif // ENVELOPECLIENT_H
