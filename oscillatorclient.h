#ifndef OSCILLATORCLIENT_H
#define OSCILLATORCLIENT_H

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

#include "parameterclient.h"
#include "oscillator.h"
#include <QGraphicsRectItem>

class OscillatorClient : public ParameterClient
{
public:
    /**
      Creates a new oscillator client object with the given name.

      This object takes ownership of the given Oscillator object, i.e., it will be deleted at destruction time.
      */
    OscillatorClient(const QString &clientName, Oscillator *oscillator, EventProcessor *eventProcessor, size_t ringBufferSize = 1024);
    virtual ~OscillatorClient();

    virtual JackClientFactory * getFactory();
private:
    Oscillator *oscillatorProcess;
};

#endif // OSCILLATORCLIENT_H
