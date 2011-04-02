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

#include "midicontroller2audioclient.h"
#include "metajack/midiport.h"
#include <QDebug>

MidiController2AudioClient::MidiController2AudioClient(const QString &clientName, unsigned char channel_, unsigned char controller_, jack_default_audio_sample_t min_, jack_default_audio_sample_t max_) :
    Midi2AudioClient(clientName),
    channel(channel_),
    controller(controller_),
    min(min_),
    max(max_),
    value(0.0),
    filter(0.001)
{
}

MidiController2AudioClient::~MidiController2AudioClient()
{
    close();
}

unsigned char MidiController2AudioClient::getChannel() const
{
    return channel;
}

unsigned char MidiController2AudioClient::getController() const
{
    return controller;
}

bool MidiController2AudioClient::init()
{
    // reset the controller value:
    value = 0;
    // reset the filter:
    filter.reset();
    return Midi2AudioClient::init();
}

bool MidiController2AudioClient::process(jack_nframes_t nframes)
{
    // get port buffers:
    void *midiInputBuffer = jack_port_get_buffer(getMidiInputPort(), nframes);
    jack_default_audio_sample_t *audioOutputBuffer = reinterpret_cast<jack_default_audio_sample_t*>(jack_port_get_buffer(getAudioOutputPort(), nframes));
    // interpret the midi input and listen to the specified controller on the given channel:
    jack_nframes_t lastFrameTime = getLastFrameTime();
    jack_nframes_t currentFrame = 0;
    jack_nframes_t currentMidiEventIndex = 0;
    jack_nframes_t midiEventCount = jack_midi_get_event_count(midiInputBuffer);
    for (; currentFrame < nframes; ) {
        // get the next midi event, if there is any:
        if (currentMidiEventIndex < midiEventCount) {
            //qDebug() << "(currentMidiEventIndex < midiEventCount) == true";
            jack_midi_event_t midiEvent;
            jack_midi_event_get(&midiEvent, midiInputBuffer, currentMidiEventIndex);
            // produce audio until the event happens:
            for (; currentFrame < midiEvent.time; currentFrame++) {
                audioOutputBuffer[currentFrame] = filter.processAudio1(value, currentFrame + lastFrameTime);
                //audioOutputBuffer[currentFrame] = value;
            }
            currentMidiEventIndex++;
            // interpret the midi event:
            unsigned char statusByte = midiEvent.buffer[0];
            unsigned char highNibble = statusByte >> 4;
            unsigned char channel = statusByte & 0x0F;
            if ((getChannel() == channel) && (highNibble == 0x0B) && midiEvent.buffer[1] == getController()) {
                value = (jack_default_audio_sample_t)midiEvent.buffer[2] * (max - min) / 127.0f + min;
                //qDebug() << value;
            }
        } else {
            // produce audio until the end of the buffer:
            for (; currentFrame < nframes; currentFrame++) {
                audioOutputBuffer[currentFrame] = filter.processAudio1(value, currentFrame + lastFrameTime);
                //audioOutputBuffer[currentFrame] = value;
            }
        }
    }
    return true;
}
