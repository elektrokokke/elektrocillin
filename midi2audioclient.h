#ifndef MIDI2AUDIOCLIENT_H
#define MIDI2AUDIOCLIENT_H

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

#include "jackclient.h"

class Midi2AudioClient : public JackClient
{
public:
    Midi2AudioClient(const QString &clientName);
    Midi2AudioClient(const QString &clientName, const QString &inputPortName, const QString &outputPortName);

    const QString & getMidiInputPortName() const;
    const QString & getAudioOutputPortName() const;

protected:
    virtual bool init();

    jack_port_t *getMidiInputPort() const;
    jack_port_t *getAudioOutputPort() const;

private:
    QString midiInputPortName, audioOutputPortName;
    jack_port_t *midiInputPort, *audioOutputPort;

};

#endif // MIDI2AUDIOCLIENT_H
