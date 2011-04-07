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

#include "midiprocessorclient.h"

MidiProcessorClient::MidiProcessorClient(const QString &clientName, AudioProcessor *audioProcessor, MidiProcessor *midiProcessor_, unsigned int channels_) :
    AudioProcessorClient(clientName, audioProcessor),
    midiProcessor(midiProcessor_),
    channels(channels_),
    midiInputPortNames(midiProcessor ? midiProcessor->getMidiInputPortNames() : QStringList()),
    midiOutputPortNames(midiProcessor ? midiProcessor->getMidiOutputPortNames() : QStringList()),
    midiInputPorts(midiInputPortNames.size()),
    midiOutputPorts(midiOutputPortNames.size())
{
    if (midiProcessor) {
        midiProcessor->setMidiWriter(this);
    }
}

MidiProcessorClient::MidiProcessorClient(const QString &clientName, const QStringList &audioInputPortNames, const QStringList &audioOutputPortNames, const QStringList &midiInputPortNames_, const QStringList &midiOutputPortNames_, unsigned int channels_) :
    AudioProcessorClient(clientName, audioInputPortNames, audioOutputPortNames),
    midiProcessor(0),
    channels(channels_),
    midiInputPortNames(midiInputPortNames_),
    midiOutputPortNames(midiOutputPortNames_),
    midiInputPorts(midiInputPortNames.size()),
    midiOutputPorts(midiOutputPortNames.size())
{
}

MidiProcessorClient::~MidiProcessorClient()
{
    close();
}

MidiProcessor * MidiProcessorClient::getMidiProcessor()
{
    return midiProcessor;
}

void MidiProcessorClient::setChannelMask(unsigned int channelMask)
{
    channels = channelMask;
}

unsigned int MidiProcessorClient::getChannelMask() const
{
    return channels;
}

bool MidiProcessorClient::init()
{
    bool ok = true;
    // create midi input and output ports:
    for (int i = 0; ok && (i < midiInputPortNames.size()); i++) {
        ok = ok && (midiInputPorts[i].port = registerMidiPort(midiInputPortNames[i], JackPortIsInput));
    }
    for (int i = 0; ok && (i < midiOutputPortNames.size()); i++) {
        ok = ok && (midiOutputPorts[i].port = registerMidiPort(midiOutputPortNames[i], JackPortIsOutput));
    }
    return ok && AudioProcessorClient::init();
}

bool MidiProcessorClient::process(jack_nframes_t nframes)
{
    // get port buffers:
    getAudioPortBuffers(nframes);
    getMidiPortBuffers(nframes);
    // process all MIDI events:
    processMidi(0, nframes);
    return true;
}

void MidiProcessorClient::processMidi(jack_nframes_t start, jack_nframes_t end)
{
    // only process the MIDI events between start and end:
    for (jack_nframes_t currentFrame = start; currentFrame < end; ) {
        // get the first midi event to process:
        int midiEventPortIndex = 0;
        jack_nframes_t midiEventTime = nframes;
        for (int i = 0; i < midiInputPorts.size(); i++) {
            const MidiInputPort &port = midiInputPorts[i];
            if (port.time < midiEventTime) {
                midiEventTime = port.time;
                midiEventPortIndex = i;
            }
        }
        if (midiEventTime < end) {
            MidiInputPort &port = midiInputPorts[midiEventPortIndex];
            if (midiEventTime >= start) {
                // produce audio until the event happens:
                processAudio(currentFrame, midiEventTime);
                currentFrame = midiEventTime;
                processMidi(midiEventPortIndex, port.event, currentFrame);
            }
            port.eventIndex++;
            // get the next midi event from that port:
            if (port.eventIndex < port.eventCount) {
                jack_midi_event_t jackMidiEvent;
                jack_midi_event_get(&jackMidiEvent, port.buffer, port.eventIndex);
                Q_ASSERT(jackMidiEvent.size <= 3);
                port.time = jackMidiEvent.time;
                port.event.size = jackMidiEvent.size;
                memcpy(port.event.buffer, jackMidiEvent.buffer, jackMidiEvent.size * sizeof(jack_midi_data_t));
            } else {
                port.time = nframes;
            }
        } else {
            // produce audio until the end of the buffer:
            processAudio(currentFrame, end);
            currentFrame = end;
        }
    }
}

void MidiProcessorClient::processMidi(int inputIndex, const MidiProcessor::MidiEvent &midiEvent, jack_nframes_t time)
{
    // interpret the midi event:
    unsigned char statusByte = midiEvent.buffer[0];
    unsigned char highNibble = statusByte >> 4;
    unsigned char channel = statusByte & 0x0F;
    // only hand down the midi event if it has the right channel:
    if ((channels >> channel) & 1) {
        if (highNibble == 0x08) {
            unsigned char noteNumber = midiEvent.buffer[1];
            unsigned char velocity = midiEvent.buffer[2];
            processNoteOff(inputIndex, channel, noteNumber, velocity, time);
        } else if (highNibble == 0x09) {
            unsigned char noteNumber = midiEvent.buffer[1];
            unsigned char velocity = midiEvent.buffer[2];
            if (velocity) {
                // note on event:
                processNoteOn(inputIndex, channel, noteNumber, velocity, time);
            } else {
                // note off event:
                processNoteOff(inputIndex, channel, noteNumber, velocity, time);
            }
        } else if (highNibble == 0x0A) {
           // aftertouch:
           unsigned char note = midiEvent.buffer[1];
           unsigned char pressure = midiEvent.buffer[2];
           processAfterTouch(inputIndex, channel, note, pressure, time);
        } else if (highNibble == 0x0B) {
            // control change:
            unsigned char controller = midiEvent.buffer[1];
            unsigned char value = midiEvent.buffer[2];
            processController(inputIndex, channel, controller, value, time);
        } else if (highNibble == 0x0E) {
            // pitch wheel:
            unsigned char low = midiEvent.buffer[1];
            unsigned char high = midiEvent.buffer[2];
            unsigned int pitch = (high << 7) + low;
            processPitchBend(inputIndex, channel, pitch, time);
        } else if (highNibble == 0x0D) {
            // channel pressure:
            unsigned char pressure = midiEvent.buffer[1];
            processChannelPressure(inputIndex, channel, pressure, time);
        }
    }
}

void MidiProcessorClient::processNoteOn(int inputIndex, unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time)
{
    Q_ASSERT(midiProcessor);
    midiProcessor->processNoteOn(inputIndex, channel, noteNumber, velocity, time);
}

void MidiProcessorClient::processNoteOff(int inputIndex, unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time)
{
    Q_ASSERT(midiProcessor);
    midiProcessor->processNoteOff(inputIndex, channel, noteNumber, velocity, time);
}

void MidiProcessorClient::processAfterTouch(int inputIndex, unsigned char channel, unsigned char noteNumber, unsigned char pressure, jack_nframes_t time)
{
    Q_ASSERT(midiProcessor);
    midiProcessor->processAfterTouch(inputIndex, channel, noteNumber, pressure, time);
}

void MidiProcessorClient::processController(int inputIndex, unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time)
{
    Q_ASSERT(midiProcessor);
    midiProcessor->processController(inputIndex, channel, controller, value, time);
}

void MidiProcessorClient::processPitchBend(int inputIndex, unsigned char channel, unsigned int value, jack_nframes_t time)
{
    Q_ASSERT(midiProcessor);
    midiProcessor->processPitchBend(inputIndex, channel, value, time);
}

void MidiProcessorClient::processChannelPressure(int inputIndex, unsigned char channel, unsigned char pressure, jack_nframes_t time)
{
    Q_ASSERT(midiProcessor);
    midiProcessor->processChannelPressure(inputIndex, channel, pressure, time);
}

void MidiProcessorClient::writeMidi(int outputIndex, const MidiProcessor::MidiEvent &event, jack_nframes_t time)
{
    Q_ASSERT((outputIndex >= 0) && (outputIndex < midiOutputPorts.size()));
    Q_ASSERT(time >= 0);
    Q_ASSERT(time < nframes);
    // write the event to the jack midi output buffer:
    jack_midi_event_write(midiOutputPorts[outputIndex].buffer, time, event.buffer, event.size * sizeof(jack_midi_data_t));
}

void MidiProcessorClient::getMidiPortBuffers(jack_nframes_t nframes)
{
    // get midi port buffers:
    for (int i = 0; i < midiInputPorts.size(); i++) {
        MidiInputPort &port = midiInputPorts[i];
        port.buffer = jack_port_get_buffer(port.port, nframes);
        port.eventIndex = 0;
        port.eventCount = jack_midi_get_event_count(port.buffer);
        // get the first midi events from each buffer, if there are any:
        if (port.eventIndex < port.eventCount) {
            jack_midi_event_t jackMidiEvent;
            jack_midi_event_get(&jackMidiEvent, port.buffer, port.eventIndex);
            Q_ASSERT(jackMidiEvent.size <= 3);
            port.time = jackMidiEvent.time;
            port.event.size = jackMidiEvent.size;
            memcpy(port.event.buffer, jackMidiEvent.buffer, jackMidiEvent.size * sizeof(jack_midi_data_t));
        } else {
            port.time = nframes;
        }
    }
    for (int i = 0; i < midiOutputPorts.size(); i++) {
        MidiOutputPort &port = midiOutputPorts[i];
        port.buffer = jack_port_get_buffer(port.port, nframes);
        // clear the buffer:
        jack_midi_clear_buffer(port.buffer);
    }
    this->nframes = nframes;
}
