#include "midisignalclient.h"

MidiSignalThread::MidiSignalThread(MidiSignalClient *client, QObject *parent) :
    JackThread(client, parent),
    ringBufferFromClient(0)
{
}

MidiSignalClient * MidiSignalThread::getMidiSignalClient()
{
    return (MidiSignalClient*)getClient();
}

void MidiSignalThread::setRingBufferFromClient(JackRingBuffer<MidiProcessorClient::MidiEvent> *ringBufferFromClient)
{
    this->ringBufferFromClient = ringBufferFromClient;
}

void MidiSignalThread::processDeferred()
{
    for (; ringBufferFromClient->readSpace(); ) {
        MidiProcessorClient::MidiEvent event = ringBufferFromClient->read();
        // interpret the MIDI event:
        unsigned char statusByte = event.buffer[0];
        unsigned char highNibble = statusByte >> 4;
        if ((highNibble >= 0x08) && (highNibble <= 0x0E)) {
            // this is a "voice" message...
            unsigned char channel = statusByte & 0x0F;
            if (highNibble == 0x08) {
                // note off message:
                unsigned char note = event.buffer[1];
                unsigned char velocity = event.buffer[2];
                // send a note off signal:
                receivedNoteOff(channel, note, velocity);
            } else if (highNibble == 0x09) {
                // note on message, but might actually be a note off (if velocity == 0):
                unsigned char note = event.buffer[1];
                unsigned char velocity = event.buffer[2];
                if (velocity) {
                    // note on event:
                    receivedNoteOn(channel, note, velocity);
                } else {
                    // note off event:
                    receivedNoteOff(channel, note, velocity);
                }
            } else if (highNibble == 0x0A) {
                // aftertouch:
                unsigned char note = event.buffer[1];
                unsigned char pressure = event.buffer[2];
                receivedAfterTouch(channel, note, pressure);
            } else if (highNibble == 0x0B) {
                // control change:
                unsigned char controller = event.buffer[1];
                unsigned char value = event.buffer[2];
                receivedControlChange(channel, controller, value);
            } else if (highNibble == 0x0C) {
                // program change:
                unsigned char program = event.buffer[1];
                receivedProgramChange(channel, program);
            } else if (highNibble == 0x0D) {
                // channel pressure:
                unsigned char pressure = event.buffer[1];
                receivedChannelPressure(channel, pressure);
            } else if (highNibble == 0x0E) {
                // pitch wheel:
                unsigned char low = event.buffer[1];
                unsigned char high = event.buffer[2];
                unsigned int pitch = (high << 7) + low;
                receivedPitchWheel(channel, pitch);
            }
        }
    }
}

void MidiSignalThread::sendNoteOff(unsigned char channel, unsigned char note, unsigned char velocity)
{
    if (getMidiSignalClient()->isActive()) {
        // create the midi message:
        MidiProcessorClient::MidiEvent event;
        event.size = 3;
        event.buffer[0] = 0x80 + channel;
        event.buffer[1] = note;
        event.buffer[2] = velocity;
        getMidiSignalClient()->postEvent(event);
    }
}

void MidiSignalThread::sendNoteOn(unsigned char channel, unsigned char note, unsigned char velocity)
{
    if (getMidiSignalClient()->isActive()) {
        // create the midi message:
        MidiProcessorClient::MidiEvent event;
        event.size = 3;
        event.buffer[0] = 0x90 + channel;
        event.buffer[1] = note;
        event.buffer[2] = velocity;
        getMidiSignalClient()->postEvent(event);
    }
}

void MidiSignalThread::sendAfterTouch(unsigned char channel, unsigned char note, unsigned char pressure)
{
    if (getMidiSignalClient()->isActive()) {
        // create the midi message:
        MidiProcessorClient::MidiEvent event;
        event.size = 3;
        event.buffer[0] = 0xA0 + channel;
        event.buffer[1] = note;
        event.buffer[2] = pressure;
        getMidiSignalClient()->postEvent(event);
    }
}

void MidiSignalThread::sendControlChange(unsigned char channel, unsigned char controller, unsigned char value)
{
    if (getMidiSignalClient()->isActive()) {
        // create the midi message:
        MidiProcessorClient::MidiEvent event;
        event.size = 3;
        event.buffer[0] = 0xB0 + channel;
        event.buffer[1] = controller;
        event.buffer[2] = value;
        getMidiSignalClient()->postEvent(event);
    }
}

void MidiSignalThread::sendProgramChange(unsigned char channel, unsigned char program)
{
    if (getMidiSignalClient()->isActive()) {
        // create the midi message:
        MidiProcessorClient::MidiEvent event;
        event.size = 2;
        event.buffer[0] = 0xC0 + channel;
        event.buffer[1] = program;
        getMidiSignalClient()->postEvent(event);
    }
}

void MidiSignalThread::sendChannelPressure(unsigned char channel, unsigned char pressure)
{
    if (getMidiSignalClient()->isActive()) {
        // create the midi message:
        MidiProcessorClient::MidiEvent event;
        event.size = 2;
        event.buffer[0] = 0xD0 + channel;
        event.buffer[1] = pressure;
        getMidiSignalClient()->postEvent(event);
    }
}

void MidiSignalThread::sendPitchWheel(unsigned char channel, unsigned int pitch)
{
    if (getMidiSignalClient()->isActive()) {
        // create the midi message:
        MidiProcessorClient::MidiEvent event;
        event.size = 3;
        event.buffer[0] = 0xE0 + channel;
        event.buffer[1] = pitch & 0x0F;
        event.buffer[2] = pitch >> 7;
        getMidiSignalClient()->postEvent(event);
    }
}

MidiSignalClient::MidiSignalClient(const QString &clientName, size_t ringBufferSize) :
    JackThreadEventProcessorClient<MidiProcessorClient::MidiEvent>(new MidiSignalThread(this), clientName, QStringList(), QStringList(), ringBufferSize),
    ringBufferToThread(ringBufferSize)
{
    getMidiSignalThread()->setRingBufferFromClient(&ringBufferToThread);
}

MidiSignalClient::~MidiSignalClient()
{
    close();
    delete getMidiSignalThread();
}

MidiSignalThread * MidiSignalClient::getMidiSignalThread()
{
    return (MidiSignalThread*)getJackThread();
}

void MidiSignalClient::processEvent(const MidiProcessorClient::MidiEvent &event, jack_nframes_t time)
{
    // write this event to the MIDI output buffer:
    writeMidi(event, time);
}

void MidiSignalClient::processMidi(const MidiProcessorClient::MidiEvent &event, jack_nframes_t)
{
    // notify the associated thread:
    ringBufferToThread.write(event);
    wakeJackThread();
}


MidiSignalThreadOld::MidiSignalThreadOld(const QString &clientName, QObject *parent) :
    JackThread(&client, parent),
    client(clientName, this),
    ringBufferFromClient(1024),
    ringBufferToClient(1024)
{
}

JackRingBuffer<MidiEventWithTimeStamp> * MidiSignalThreadOld::getInputRingBuffer()
{
    return &ringBufferFromClient;
}

JackRingBuffer<MidiEventWithTimeStamp> * MidiSignalThreadOld::getOutputRingBuffer()
{
    return &ringBufferToClient;
}

void MidiSignalThreadOld::processDeferred()
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

void MidiSignalThreadOld::sendNoteOff(unsigned char channel, unsigned char note, unsigned char velocity)
{
    if (client.isActive()) {
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
}

void MidiSignalThreadOld::sendNoteOn(unsigned char channel, unsigned char note, unsigned char velocity)
{
    if (client.isActive()) {
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
    }
}

void MidiSignalThreadOld::sendAfterTouch(unsigned char channel, unsigned char note, unsigned char pressure)
{
    if (client.isActive()) {
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
}

void MidiSignalThreadOld::sendControlChange(unsigned char channel, unsigned char controller, unsigned char value)
{
    if (client.isActive()) {
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
}

void MidiSignalThreadOld::sendProgramChange(unsigned char channel, unsigned char program)
{
    if (client.isActive()) {
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
}

void MidiSignalThreadOld::sendChannelPressure(unsigned char channel, unsigned char pressure)
{
    if (client.isActive()) {
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
}

void MidiSignalThreadOld::sendPitchWheel(unsigned char channel, unsigned int pitch)
{
    if (client.isActive()) {
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
}

MidiSignalThreadOld::MidiSignalClientOld::MidiSignalClientOld(const QString &clientName, JackThread *thread) :
        JackClientWithDeferredProcessing(clientName, thread),
        midiInputPortName("Midi in"),
        midiOutputPortName("Midi out")
{
}

MidiSignalThreadOld::MidiSignalClientOld::~MidiSignalClientOld()
{
    // close the client (this waits for the associated thread to finish):
    close();
}

const QString & MidiSignalThreadOld::MidiSignalClientOld::getMidiInputPortName() const
{
    return midiInputPortName;
}

const QString & MidiSignalThreadOld::MidiSignalClientOld::getMidiOutputPortName() const
{
    return midiOutputPortName;
}

MidiSignalThreadOld * MidiSignalThreadOld::MidiSignalClientOld::getMidiThread()
{
    return (MidiSignalThreadOld*)getJackThread();
}

JackRingBuffer<MidiEventWithTimeStamp> * MidiSignalThreadOld::MidiSignalClientOld::getInputRingBuffer()
{
    return getMidiThread()->getOutputRingBuffer();
}

JackRingBuffer<MidiEventWithTimeStamp> * MidiSignalThreadOld::MidiSignalClientOld::getOutputRingBuffer()
{
    return getMidiThread()->getInputRingBuffer();
}

bool MidiSignalThreadOld::MidiSignalClientOld::init()
{
    // setup the input and output midi ports:
    midiIn = registerMidiPort(midiInputPortName, JackPortIsInput);
    midiOut = registerMidiPort(midiOutputPortName, JackPortIsOutput);
    return (JackClientWithDeferredProcessing::init() && midiIn && midiOut);
}

bool MidiSignalThreadOld::MidiSignalClientOld::process(jack_nframes_t nframes)
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
