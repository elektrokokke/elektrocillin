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

#include "oscillatorclient.h"
#include "graphicsdiscretecontrolitem.h"
#include <QPen>
#include <QBrush>

OscillatorClient::OscillatorClient(const QString &clientName, Oscillator *oscillator, EventProcessor *eventProcessor, size_t ringBufferSize) :
    ParameterClient(clientName, oscillator, oscillator, eventProcessor, oscillator, ringBufferSize),
    oscillatorProcess(oscillator)
{
}

OscillatorClient::~OscillatorClient()
{
    close();
    delete oscillatorProcess;
}

class OscillatorClientFactory : public JackClientFactory
{
public:
    OscillatorClientFactory()
    {
        JackClientSerializer::getInstance()->registerFactory(this);
    }
    QString getName()
    {
        return "Sine oscillator";
    }
    JackClient * createClient(const QString &clientName)
    {
        return new OscillatorClient(clientName, new Oscillator(), 0);
    }
    static OscillatorClientFactory factory;
};

OscillatorClientFactory OscillatorClientFactory::factory;

JackClientFactory * OscillatorClient::getFactory()
{
    return &OscillatorClientFactory::factory;
}
