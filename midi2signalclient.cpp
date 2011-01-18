#include "midi2signalclient.h"
#include <QDebug>

const size_t Midi2SignalClient::ringBufferSize = 4096;

Midi2SignalClient::Midi2SignalClient(const QString &clientName, QObject *parent) :
    QThread(parent),
    JackClient(clientName)
{
    // create the ring buffers:
    ringBufferIn = jack_ringbuffer_create(ringBufferSize);
    ringBufferOut = jack_ringbuffer_create(ringBufferSize);
}

Midi2SignalClient::~Midi2SignalClient()
{
    if (ringBufferIn) {
        jack_ringbuffer_free(ringBufferIn);
    }
    if (ringBufferOut) {
        jack_ringbuffer_free(ringBufferOut);
    }
}

bool Midi2SignalClient::setup()
{
    // start the QThread:
    if (!isRunning()) {
        start();
    }
    // setup the input and output midi ports:
    midiIn = registerMidiPort("midi in", JackPortIsInput);
    midiOut = registerMidiPort("midi out", JackPortIsOutput);
    return (ringBufferIn && ringBufferOut && midiIn && midiOut);
}

bool Midi2SignalClient::process(jack_nframes_t nframes)
{
    // get port buffers:
    void *midiInputBuffer = jack_port_get_buffer(midiIn, nframes);
    void *midiOutputBuffer = jack_port_get_buffer(midiOut, nframes);
    // clear the output buffer (important!):
    jack_midi_clear_buffer(midiOutputBuffer);
    // get the start time of the current process cycle:
    jack_nframes_t lastFrameTime = getLastFrameTime();
    // put the midi input into the input ringbuffer:
    jack_nframes_t midiInCount = jack_midi_get_event_count(midiInputBuffer);
    int messagesWritten = 0;
    for (jack_nframes_t midiInIndex = 0; midiInIndex < midiInCount; midiInIndex++) {
        jack_midi_event_t midiEvent;
        jack_midi_event_get(&midiEvent, midiInputBuffer, midiInIndex);
        // midi message with more than 3 bytes are currently not supported:
        if (midiEvent.size <= 3) {
            // write the midi event to the ring buffer:
            if (jack_ringbuffer_write_space(ringBufferIn) >= sizeof(MidiMessage)) {
                MidiMessage message;
                // convert the time from current frame base to "global" time:
                message.time = lastFrameTime + midiEvent.time;
                message.size = midiEvent.size;
                memcpy(message.message, midiEvent.buffer, message.size);
                jack_ringbuffer_write(ringBufferIn, (const char*)&message, sizeof(MidiMessage));
                messagesWritten++;
            } else {
                // TODO: ignore lost midi events for now...
            }
        }
    }
    // tell the QThread to wake up and convert the midi input to signals:
    if (messagesWritten) {
        waitForMidi.wakeOne();
    }
    // now read from midi output ring buffer and write to the jack midi output buffer:
    for (bool hasEvents = true; hasEvents; ) {
        hasEvents = false;
        if (jack_ringbuffer_read_space(ringBufferOut) >= sizeof(MidiMessage)) {
            // get the midi message from the ring buffer:
            MidiMessage message;
            jack_ringbuffer_peek(ringBufferOut, (char*)&message, sizeof(MidiMessage));
            // adjust time relative to the beginning of this frame:
            if (message.time + nframes < lastFrameTime) {
                // if time is too early, this is in the buffer for too long, adjust time accordingly:
                message.time = 0;
                //qDebug() << "time had to be adjusted to zero";
            } else {
                message.time = message.time + nframes - lastFrameTime;
            }
            // test if the event belongs into this frame (and not the next):
            if (message.time < nframes) {
                // write the event to the jack midi output buffer:
                if (jack_midi_event_write(midiOutputBuffer, message.time, message.message, message.size)) {
                    // could not write the event...
                    //qDebug() << "midi output buffer full";
                } else {
                    jack_ringbuffer_read_advance(ringBufferOut, sizeof(MidiMessage));
                    //qDebug() << "wrote midi message at time" << timeAndSize.time << "with size" << timeAndSize.size << "in buffer" << buffer;
                    hasEvents = true;
                }
            }
        }
    }
    return true;
}

void Midi2SignalClient::run()
{
    // mutex has to be locked to be used for the wait condition:
    mutexForMidi.lock();
    for (; true; ) {
        // wait for midi input:
        waitForMidi.wait(&mutexForMidi);
        // read midi input from the ring buffer:
        for (bool hasEvents = true; hasEvents;) {
            hasEvents = false;
            if (jack_ringbuffer_read_space(ringBufferIn) >= sizeof(MidiMessage)) {
                // get the midi event from the ring buffer:
                MidiMessage message;
                jack_ringbuffer_read(ringBufferIn, (char*)&message, sizeof(MidiMessage));
                hasEvents = true;
                // interpret the midi event:
                unsigned char statusByte = message.message[0];
                unsigned char highNibble = statusByte >> 4;
                if ((highNibble >= 0x08) && (highNibble <= 0x0E)) {
                    // this is a "voice" message...
                    unsigned char channel = statusByte & 0x0F;
                    if (highNibble == 0x08) {
                        // note off message:
                        unsigned char note = message.message[1];
                        unsigned char velocity = message.message[2];
                        // send a note off signal:
                        receivedNoteOff(channel, note, velocity);
                    } else if (highNibble == 0x09) {
                        // note on message, but might actually be a note off (if velocity == 0):
                        unsigned char note = message.message[1];
                        unsigned char velocity = message.message[2];
                        if (velocity) {
                            // note on event:
                            receivedNoteOn(channel, note, velocity);
                        } else {
                            // note off event:
                            receivedNoteOff(channel, note, velocity);
                        }
                    } else if (highNibble == 0x0A) {
                        // aftertouch:
                        unsigned char note = message.message[1];
                        unsigned char pressure = message.message[2];
                        receivedAfterTouch(channel, note, pressure);
                    } else if (highNibble == 0x0B) {
                        // control change:
                        unsigned char controller = message.message[1];
                        unsigned char value = message.message[2];
                        receivedControlChange(channel, controller, value);
                    } else if (highNibble == 0x0C) {
                        // program change:
                        unsigned char program = message.message[1];
                        receivedProgramChange(channel, program);
                    } else if (highNibble == 0x0D) {
                        // channel pressure:
                        unsigned char pressure = message.message[1];
                        receivedChannelPressure(channel, pressure);
                    } else if (highNibble == 0x0E) {
                        // pitch wheel:
                        unsigned char low = message.message[1];
                        unsigned char high = message.message[2];
                        unsigned int pitch = (high << 7) + low;
                        receivedPitchWheel(channel, pitch);
                    }
                }
            }
        }
    }
}

void Midi2SignalClient::writeMidiEventToOutputRingBuffer(const MidiMessage &message)
{
    // write the midi event to the output ring buffer:
    if (jack_ringbuffer_write_space(ringBufferOut) >= sizeof(MidiMessage)) {
        jack_ringbuffer_write(ringBufferOut, (const char*)&message, sizeof(MidiMessage));
    } else {
        // TODO: ignore lost midi events for now...
    }
}

void Midi2SignalClient::sendNoteOff(unsigned char channel, unsigned char note, unsigned char velocity)
{
    // create the midi message:
    MidiMessage message;
    message.size = 3;
    message.message[0] = 0x80 + channel;
    message.message[1] = note;
    message.message[2] = velocity;
    // get estimated current time:
    message.time = getEstimatedCurrentTime();
    // send the midi message:
    writeMidiEventToOutputRingBuffer(message);
}

void Midi2SignalClient::sendNoteOn(unsigned char channel, unsigned char note, unsigned char velocity)
{
    // note on with velocity 0 is actually a note off:
    if (velocity == 0) {
        sendNoteOff(channel, note, velocity);
    } else {
        // create the midi message:
        MidiMessage message;
        message.size = 3;
        message.message[0] = 0x90 + channel;
        message.message[1] = note;
        message.message[2] = velocity;
        // get estimated current time:
        message.time = getEstimatedCurrentTime();
        // send the midi message:
        writeMidiEventToOutputRingBuffer(message);
    }
}

void Midi2SignalClient::sendAfterTouch(unsigned char channel, unsigned char note, unsigned char pressure)
{
    // create the midi message:
    MidiMessage message;
    message.size = 3;
    message.message[0] = 0xA0 + channel;
    message.message[1] = note;
    message.message[2] = pressure;
    // get estimated current time:
    message.time = getEstimatedCurrentTime();
    // send the midi message:
    writeMidiEventToOutputRingBuffer(message);
}

void Midi2SignalClient::sendControlChange(unsigned char channel, unsigned char controller, unsigned char value)
{
    // create the midi message:
    MidiMessage message;
    message.size = 3;
    message.message[0] = 0xB0 + channel;
    message.message[1] = controller;
    message.message[2] = value;
    // get estimated current time:
    message.time = getEstimatedCurrentTime();
    // send the midi message:
    writeMidiEventToOutputRingBuffer(message);
}

void Midi2SignalClient::sendProgramChange(unsigned char channel, unsigned char program)
{
    // create the midi message:
    MidiMessage message;
    message.size = 2;
    message.message[0] = 0xC0 + channel;
    message.message[1] = program;
    // get estimated current time:
    message.time = getEstimatedCurrentTime();
    // send the midi message:
    writeMidiEventToOutputRingBuffer(message);
}

void Midi2SignalClient::sendChannelPressure(unsigned char channel, unsigned char pressure)
{
    // create the midi message:
    MidiMessage message;
    message.size = 2;
    message.message[0] = 0xD0 + channel;
    message.message[1] = pressure;
    // get estimated current time:
    message.time = getEstimatedCurrentTime();

    // send the midi message:
    writeMidiEventToOutputRingBuffer(message);
}

void Midi2SignalClient::sendPitchWheel(unsigned char channel, unsigned int pitch)
{
    // create the midi message:
    MidiMessage message;
    message.size = 3;
    message.message[0] = 0xE0 + channel;
    message.message[1] = pitch & 0x0F;
    message.message[2] = pitch >> 7;
    // get estimated current time:
    message.time = getEstimatedCurrentTime();
    // send the midi message:
    writeMidiEventToOutputRingBuffer(message);
}
