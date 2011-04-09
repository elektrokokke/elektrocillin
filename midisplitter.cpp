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

#include "midisplitter.h"

ControlChange::ControlChange(QObject *parent) :
    QObject(parent)
{
}

void ControlChange::onControlChange(unsigned char channel, unsigned char controller, unsigned char value)
{
    receivedControlChange(channel, controller, value);
}

MidiSplitter::MidiSplitter(QObject *parent) :
    QObject(parent)
{
}

ControlChange * MidiSplitter::getControlChangeSink(unsigned char controller)
{
    // create a new sink, if necessary:
    if (!mapControllerToControlChange.contains(controller)) {
        mapControllerToControlChange.insert(controller, new ControlChange(this));
    }
    return mapControllerToControlChange.value(controller);
}

void MidiSplitter::splitControlChangeByController(unsigned char channel, unsigned char controller, unsigned char value)
{
    // get the corresponding ControlChange object:
    ControlChange *controlChange = mapControllerToControlChange.value(controller, 0);
    if (controlChange) {
        controlChange->onControlChange(channel, controller, value);
    }
}
