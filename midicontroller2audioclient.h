#ifndef MIDICONTROLLER2AUDIOCLIENT_H
#define MIDICONTROLLER2AUDIOCLIENT_H

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

#include "midi2audioclient.h"
#include "butterworth2polefilter.h"

class MidiController2AudioClient : public Midi2AudioClient
{
public:
    MidiController2AudioClient(const QString &clientName, unsigned char channel, unsigned char controller, jack_default_audio_sample_t min = -1.0f, jack_default_audio_sample_t max = 1.0f);
    ~MidiController2AudioClient();

    unsigned char getChannel() const;
    unsigned char getController() const;

protected:
    virtual bool init();
    virtual bool process(jack_nframes_t nframes);

private:
    unsigned char channel, controller;
    jack_default_audio_sample_t min, max, value;
    Butterworth2PoleFilter filter;
};

#endif // MIDICONTROLLER2AUDIOCLIENT_H
