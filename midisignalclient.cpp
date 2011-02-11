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
    activateMidiOutput(true);
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
