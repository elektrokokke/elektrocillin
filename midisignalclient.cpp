#include "midisignalclient.h"

MidiSignalThread::MidiSignalThread(const QString &clientName, QObject *parent) :
    JackThread(&client, parent),
    client(clientName, this),
    ringBufferFromClient(1024),
    ringBufferToClient(1024)
{
}

JackRingBuffer<MidiEventWithTimeStamp> * MidiSignalThread::getInputRingBuffer()
{
    return &ringBufferFromClient;
}

JackRingBuffer<MidiEventWithTimeStamp> * MidiSignalThread::getOutputRingBuffer()
{
    return &ringBufferToClient;
}

void MidiSignalThread::processDeferred()
{
    // read midi input from the ring buffer:
    for (; getInputRingBuffer()->readSpace(); ) {
        // get the midi event from the ring buffer:
        MidiEventWithTimeStamp eventWithTimeStamp = getInputRingBuffer()->read();
        // interpret the midi event:
        unsigned char statusByte = eventWithTimeStamp.event.buffer[0];
        unsigned char highNibble = statusByte >> 4;
        if ((highNibble >= 0x08) && (highNibble <= 0x0E)) {
            // this is a "voice" message...
            unsigned char channel = statusByte & 0x0F;
            if (highNibble == 0x08) {
                // note off message:
                unsigned char note = eventWithTimeStamp.event.buffer[1];
                unsigned char velocity = eventWithTimeStamp.event.buffer[2];
                // send a note off signal:
                receivedNoteOff(channel, note, velocity);
            } else if (highNibble == 0x09) {
                // note on message, but might actually be a note off (if velocity == 0):
                unsigned char note = eventWithTimeStamp.event.buffer[1];
                unsigned char velocity = eventWithTimeStamp.event.buffer[2];
                if (velocity) {
                    // note on event:
                    receivedNoteOn(channel, note, velocity);
//                    qDebug() << "receivedNoteOn(" << channel << "," << note << "," << velocity << ") at time" << message.time << "(buffer time" << message.bufferTime << ")";
                } else {
                    // note off event:
                    receivedNoteOff(channel, note, velocity);
                }
            } else if (highNibble == 0x0A) {
                // aftertouch:
                unsigned char note = eventWithTimeStamp.event.buffer[1];
                unsigned char pressure = eventWithTimeStamp.event.buffer[2];
                receivedAfterTouch(channel, note, pressure);
            } else if (highNibble == 0x0B) {
                // control change:
                unsigned char controller = eventWithTimeStamp.event.buffer[1];
                unsigned char value = eventWithTimeStamp.event.buffer[2];
                receivedControlChange(channel, controller, value);
//                qDebug() << "receivedControlChange(" << channel << "," << controller << "," << value << ") at time" << message.time << "(buffer time" << message.bufferTime << ")";
            } else if (highNibble == 0x0C) {
                // program change:
                unsigned char program = eventWithTimeStamp.event.buffer[1];
                receivedProgramChange(channel, program);
            } else if (highNibble == 0x0D) {
                // channel pressure:
                unsigned char pressure = eventWithTimeStamp.event.buffer[1];
                receivedChannelPressure(channel, pressure);
            } else if (highNibble == 0x0E) {
                // pitch wheel:
                unsigned char low = eventWithTimeStamp.event.buffer[1];
                unsigned char high = eventWithTimeStamp.event.buffer[2];
                unsigned int pitch = (high << 7) + low;
                receivedPitchWheel(channel, pitch);
            }
        }
    }
}

void MidiSignalThread::sendNoteOff(unsigned char channel, unsigned char note, unsigned char velocity)
{
    // create the midi message:
    MidiEventWithTimeStamp eventWithTimeStamp;
    eventWithTimeStamp.event.size = 3;
    eventWithTimeStamp.event.buffer[0] = 0x80 + channel;
    eventWithTimeStamp.event.buffer[1] = note;
    eventWithTimeStamp.event.buffer[2] = velocity;
    // get estimated current time:
    eventWithTimeStamp.time = getClient()->getEstimatedCurrentTime();
    // send the midi message:
    getOutputRingBuffer()->write(eventWithTimeStamp);
}

void MidiSignalThread::sendNoteOn(unsigned char channel, unsigned char note, unsigned char velocity)
{
    // create the midi message:
    MidiEventWithTimeStamp eventWithTimeStamp;
    eventWithTimeStamp.event.size = 3;
    eventWithTimeStamp.event.buffer[0] = 0x90 + channel;
    eventWithTimeStamp.event.buffer[1] = note;
    eventWithTimeStamp.event.buffer[2] = velocity;
    // get estimated current time:
    eventWithTimeStamp.time = getClient()->getEstimatedCurrentTime();
    // send the midi message:
    getOutputRingBuffer()->write(eventWithTimeStamp);
//    qDebug() << "sendNoteOn(" << channel << "," << note << "," << velocity << ") at time" << message.time;
}

void MidiSignalThread::sendAfterTouch(unsigned char channel, unsigned char note, unsigned char pressure)
{
    // create the midi message:
    MidiEventWithTimeStamp eventWithTimeStamp;
    eventWithTimeStamp.event.size = 3;
    eventWithTimeStamp.event.buffer[0] = 0xA0 + channel;
    eventWithTimeStamp.event.buffer[1] = note;
    eventWithTimeStamp.event.buffer[2] = pressure;
    // get estimated current time:
    eventWithTimeStamp.time = getClient()->getEstimatedCurrentTime();
    // send the midi message:
    getOutputRingBuffer()->write(eventWithTimeStamp);
}

void MidiSignalThread::sendControlChange(unsigned char channel, unsigned char controller, unsigned char value)
{
    // create the midi message:
    MidiEventWithTimeStamp eventWithTimeStamp;
    eventWithTimeStamp.event.size = 3;
    eventWithTimeStamp.event.buffer[0] = 0xB0 + channel;
    eventWithTimeStamp.event.buffer[1] = controller;
    eventWithTimeStamp.event.buffer[2] = value;
    // get estimated current time:
    eventWithTimeStamp.time = getClient()->getEstimatedCurrentTime();
    // send the midi message:
    getOutputRingBuffer()->write(eventWithTimeStamp);
}

void MidiSignalThread::sendProgramChange(unsigned char channel, unsigned char program)
{
    // create the midi message:
    MidiEventWithTimeStamp eventWithTimeStamp;
    eventWithTimeStamp.event.size = 2;
    eventWithTimeStamp.event.buffer[0] = 0xC0 + channel;
    eventWithTimeStamp.event.buffer[1] = program;
    // get estimated current time:
    eventWithTimeStamp.time = getClient()->getEstimatedCurrentTime();
    // send the midi message:
    getOutputRingBuffer()->write(eventWithTimeStamp);
}

void MidiSignalThread::sendChannelPressure(unsigned char channel, unsigned char pressure)
{
    // create the midi message:
    MidiEventWithTimeStamp eventWithTimeStamp;
    eventWithTimeStamp.event.size = 2;
    eventWithTimeStamp.event.buffer[0] = 0xD0 + channel;
    eventWithTimeStamp.event.buffer[1] = pressure;
    // get estimated current time:
    eventWithTimeStamp.time = getClient()->getEstimatedCurrentTime();
    // send the midi message:
    getOutputRingBuffer()->write(eventWithTimeStamp);
}

void MidiSignalThread::sendPitchWheel(unsigned char channel, unsigned int pitch)
{
    // create the midi message:
    MidiEventWithTimeStamp eventWithTimeStamp;
    eventWithTimeStamp.event.size = 3;
    eventWithTimeStamp.event.buffer[0] = 0xE0 + channel;
    eventWithTimeStamp.event.buffer[1] = pitch & 0x0F;
    eventWithTimeStamp.event.buffer[2] = pitch >> 7;
    // get estimated current time:
    eventWithTimeStamp.time = getClient()->getEstimatedCurrentTime();
    // send the midi message:
    getOutputRingBuffer()->write(eventWithTimeStamp);
}

MidiSignalThread::MidiSignalClient::MidiSignalClient(const QString &clientName, JackThread *thread) :
        JackClientWithDeferredProcessing(clientName, thread),
        midiInputPortName("midi in"),
        midiOutputPortName("midi out")
{
}

MidiSignalThread::MidiSignalClient::~MidiSignalClient()
{
    // close the client (this waits for the associated thread to finish):
    close();
}

const QString & MidiSignalThread::MidiSignalClient::getMidiInputPortName() const
{
    return midiInputPortName;
}

const QString & MidiSignalThread::MidiSignalClient::getMidiOutputPortName() const
{
    return midiOutputPortName;
}

MidiSignalThread * MidiSignalThread::MidiSignalClient::getMidiThread()
{
    return (MidiSignalThread*)getJackThread();
}

JackRingBuffer<MidiEventWithTimeStamp> * MidiSignalThread::MidiSignalClient::getInputRingBuffer()
{
    return getMidiThread()->getOutputRingBuffer();
}

JackRingBuffer<MidiEventWithTimeStamp> * MidiSignalThread::MidiSignalClient::getOutputRingBuffer()
{
    return getMidiThread()->getInputRingBuffer();
}

bool MidiSignalThread::MidiSignalClient::init()
{
    // setup the input and output midi ports:
    midiIn = registerMidiPort(midiInputPortName, JackPortIsInput);
    midiOut = registerMidiPort(midiOutputPortName, JackPortIsOutput);
    return (JackClientWithDeferredProcessing::init() && midiIn && midiOut);
}

bool MidiSignalThread::MidiSignalClient::process(jack_nframes_t nframes)
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
        jack_midi_event_t jackMidiEvent;
        jack_midi_event_get(&jackMidiEvent, midiInputBuffer, midiInIndex);
        // midi message with more than 3 bytes are currently not supported:
        if (jackMidiEvent.size <= 3) {
            // write the midi event to the ring buffer:
            MidiEventWithTimeStamp eventWithTimeStamp;
            // convert the time from current frame base to "global" time:
            eventWithTimeStamp.time = lastFrameTime + jackMidiEvent.time;
            eventWithTimeStamp.event.size = jackMidiEvent.size;
            memcpy(eventWithTimeStamp.event.buffer, jackMidiEvent.buffer, jackMidiEvent.size * sizeof(jack_midi_data_t));
            getOutputRingBuffer()->write(eventWithTimeStamp);
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
            MidiEventWithTimeStamp eventWithTimeStamp = getInputRingBuffer()->peek();
            // adjust time relative to the beginning of this frame:
            if (eventWithTimeStamp.time + nframes < lastFrameTime) {
                // if time is too early, this is in the buffer for too long, adjust time accordingly:
                eventWithTimeStamp.time = 0;
                //qDebug() << "time had to be adjusted to zero";
            } else {
                eventWithTimeStamp.time = eventWithTimeStamp.time + nframes - lastFrameTime;
            }
            // test if the event belongs into this frame (and not the next):
            if (eventWithTimeStamp.time < nframes) {
                // write the event to the jack midi output buffer:
                jack_midi_event_write(midiOutputBuffer, eventWithTimeStamp.time, eventWithTimeStamp.event.buffer, eventWithTimeStamp.event.size * sizeof(jack_midi_data_t));
                getInputRingBuffer()->readAdvance(1);
                hasEvents = true;
            }
        }
    }
    return true;
}
