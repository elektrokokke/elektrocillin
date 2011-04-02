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

#include "midi2audioclient.h"

Midi2AudioClient::Midi2AudioClient(const QString &clientName) :
    JackClient(clientName),
    midiInputPortName("midi in"),
    audioOutputPortName("audio out")
{
}

Midi2AudioClient::Midi2AudioClient(const QString &clientName, const QString &inputPortName, const QString &outputPortName) :
    JackClient(clientName),
    midiInputPortName(inputPortName),
    audioOutputPortName(outputPortName)
{
}

const QString & Midi2AudioClient::getMidiInputPortName() const
{
    return midiInputPortName;
}

const QString & Midi2AudioClient::getAudioOutputPortName() const
{
    return audioOutputPortName;
}

bool Midi2AudioClient::init()
{
    // register input and output ports:
    midiInputPort = registerMidiPort(midiInputPortName, JackPortIsInput);
    audioOutputPort = registerAudioPort(audioOutputPortName, JackPortIsOutput);
    return midiInputPort && audioOutputPort;
}

jack_port_t * Midi2AudioClient::getMidiInputPort() const
{
    return midiInputPort;
}

jack_port_t * Midi2AudioClient::getAudioOutputPort() const
{
    return audioOutputPort;
}
