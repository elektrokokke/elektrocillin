#ifndef OSCILLATORCLIENT_H
#define OSCILLATORCLIENT_H

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

#include "eventprocessorclient.h"
#include "oscillator.h"
#include <QGraphicsRectItem>

class OscillatorClient : public EventProcessorClient
{
public:
    /**
      Creates a new oscillator client object with the given name.

      This object takes ownership of the given Oscillator object, i.e., it will be deleted at destruction time.
      */
    OscillatorClient(const QString &clientName, Oscillator *oscillator, size_t ringBufferSize = 1024);
    virtual ~OscillatorClient();

    virtual JackClientFactory * getFactory();
    virtual void saveState(QDataStream &stream);
    /**
      To call this method is only safe when the client is not running,
      as it accesses the internal Oscillator object used by the Jack
      process thread in a non-threadsafe way.

      To change the oscillator gain while the client is running use
      postChangeGain() method.
      */
    virtual void loadState(QDataStream &stream);

    Oscillator * getOscillator();

    void postChangeGain(double gain);
    void postChangeTune(double tune);
    void postChangePitchModulationIntensity(double halfTones);

    QGraphicsItem * createGraphicsItem();
private:
    Oscillator oscillator, *oscillatorProcess;
};

class OscillatorClientGraphicsItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    OscillatorClientGraphicsItem(OscillatorClient *client, QGraphicsItem *parent = 0);
protected:
    virtual void focusInEvent(QFocusEvent * event);
    virtual void focusOutEvent(QFocusEvent * event);
private slots:
    void onGainChanged(double value);
    void onDetuneChanged(double value);
    void onPitchModulationIntensityChanged(double value);
private:
    OscillatorClient *client;
};

#endif // OSCILLATORCLIENT_H
