#include "midiclient.h"

MidiThread::MidiThread(MidiClient *client, QObject *parent) :
        JackThread(client, parent),
        ringBufferFromClient(1024),
        ringBufferToClient(1024)
{
}

JackRingBuffer<MidiMessage> * MidiThread::getInputRingBuffer()
{
    return &ringBufferFromClient;
}

JackRingBuffer<MidiMessage> * MidiThread::getOutputRingBuffer()
{
    return &ringBufferToClient;
}

void MidiThread::processDeferred()
{
    // read midi input from the ring buffer:
    for (; getInputRingBuffer()->readSpace(); ) {
        // get the midi event from the ring buffer:
        MidiMessage message = getInputRingBuffer()->read();
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

void MidiThread::sendNoteOff(unsigned char channel, unsigned char note, unsigned char velocity)
{
    // create the midi message:
    MidiMessage message;
    message.size = 3;
    message.message[0] = 0x80 + channel;
    message.message[1] = note;
    message.message[2] = velocity;
    // get estimated current time:
    message.time = getClient()->getEstimatedCurrentTime();
    // send the midi message:
    getOutputRingBuffer()->write(message);
}

void MidiThread::sendNoteOn(unsigned char channel, unsigned char note, unsigned char velocity)
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
        message.time = getClient()->getEstimatedCurrentTime();
        // send the midi message:
        getOutputRingBuffer()->write(message);
    }
}

void MidiThread::sendAfterTouch(unsigned char channel, unsigned char note, unsigned char pressure)
{
    // create the midi message:
    MidiMessage message;
    message.size = 3;
    message.message[0] = 0xA0 + channel;
    message.message[1] = note;
    message.message[2] = pressure;
    // get estimated current time:
    message.time = getClient()->getEstimatedCurrentTime();
    // send the midi message:
    getOutputRingBuffer()->write(message);
}

void MidiThread::sendControlChange(unsigned char channel, unsigned char controller, unsigned char value)
{
    // create the midi message:
    MidiMessage message;
    message.size = 3;
    message.message[0] = 0xB0 + channel;
    message.message[1] = controller;
    message.message[2] = value;
    // get estimated current time:
    message.time = getClient()->getEstimatedCurrentTime();
    // send the midi message:
    getOutputRingBuffer()->write(message);
}

void MidiThread::sendProgramChange(unsigned char channel, unsigned char program)
{
    // create the midi message:
    MidiMessage message;
    message.size = 2;
    message.message[0] = 0xC0 + channel;
    message.message[1] = program;
    // get estimated current time:
    message.time = getClient()->getEstimatedCurrentTime();
    // send the midi message:
    getOutputRingBuffer()->write(message);
}

void MidiThread::sendChannelPressure(unsigned char channel, unsigned char pressure)
{
    // create the midi message:
    MidiMessage message;
    message.size = 2;
    message.message[0] = 0xD0 + channel;
    message.message[1] = pressure;
    // get estimated current time:
    message.time = getClient()->getEstimatedCurrentTime();
    // send the midi message:
    getOutputRingBuffer()->write(message);
}

void MidiThread::sendPitchWheel(unsigned char channel, unsigned int pitch)
{
    // create the midi message:
    MidiMessage message;
    message.size = 3;
    message.message[0] = 0xE0 + channel;
    message.message[1] = pitch & 0x0F;
    message.message[2] = pitch >> 7;
    // get estimated current time:
    message.time = getClient()->getEstimatedCurrentTime();
    // send the midi message:
    getOutputRingBuffer()->write(message);
}

MidiClient::MidiClient(const QString &clientName) :
        JackClientWithDeferredProcessing(clientName, &thread),
        midiInputPortName("midi in"),
        midiOutputPortName("midi out"),
        thread(this)
{
}

MidiClient::~MidiClient()
{
    // close the client (this waits for the associated thread to finish):
    close();
}

const QString & MidiClient::getMidiInputPortName() const
{
    return midiInputPortName;
}

const QString & MidiClient::getMidiOutputPortName() const
{
    return midiOutputPortName;
}

MidiThread * MidiClient::getMidiThread()
{
    return (MidiThread*)getJackThread();
}

JackRingBuffer<MidiMessage> * MidiClient::getInputRingBuffer()
{
    return getMidiThread()->getOutputRingBuffer();
}

JackRingBuffer<MidiMessage> * MidiClient::getOutputRingBuffer()
{
    return getMidiThread()->getInputRingBuffer();
}

bool MidiClient::init()
{
    // setup the input and output midi ports:
    midiIn = registerMidiPort(midiInputPortName, JackPortIsInput);
    midiOut = registerMidiPort(midiOutputPortName, JackPortIsOutput);
    return (JackClientWithDeferredProcessing::init() && midiIn && midiOut);
}

bool MidiClient::process(jack_nframes_t nframes)
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
            MidiMessage message;
            // convert the time from current frame base to "global" time:
            message.time = lastFrameTime + midiEvent.time;
            message.size = midiEvent.size;
            memcpy(message.message, midiEvent.buffer, message.size);
            getOutputRingBuffer()->write(message);
            messagesWritten++;
        }
    }
    // tell the MidiThread to wake up and convert the midi input to signals:
    if (messagesWritten) {
        wakeJackThread();
    }
    // now read from midi output ring buffer and write to the jack midi output buffer:
    for (bool hasEvents = true; hasEvents; ) {
        hasEvents = false;
        if (getInputRingBuffer()->readSpace()) {
            // get the midi message from the ring buffer:
            MidiMessage message = getInputRingBuffer()->peek();
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
                } else {
                    getInputRingBuffer()->readAdvance(1);
                    hasEvents = true;
                }
            }
        }
    }
    return true;
}
