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

void MidiSignalThread::setRingBufferFromClient(JackRingBuffer<MidiProcessor::MidiEvent> *ringBufferFromClient)
{
    this->ringBufferFromClient = ringBufferFromClient;
}

void MidiSignalThread::processDeferred()
{
    for (; ringBufferFromClient->readSpace(); ) {
        MidiProcessor::MidiEvent event = ringBufferFromClient->read();
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
        MidiProcessor::MidiEvent *event = new MidiProcessor::MidiEvent();
        event->size = 3;
        event->buffer[0] = 0x80 + channel;
        event->buffer[1] = note;
        event->buffer[2] = velocity;
        getMidiSignalClient()->postEvent(event);
    }
}

void MidiSignalThread::sendNoteOn(unsigned char channel, unsigned char note, unsigned char velocity)
{
    if (getMidiSignalClient()->isActive()) {
        // create the midi message:
        MidiProcessor::MidiEvent *event = new MidiProcessor::MidiEvent();
        event->size = 3;
        event->buffer[0] = 0x90 + channel;
        event->buffer[1] = note;
        event->buffer[2] = velocity;
        getMidiSignalClient()->postEvent(event);
    }
}

void MidiSignalThread::sendAfterTouch(unsigned char channel, unsigned char note, unsigned char pressure)
{
    if (getMidiSignalClient()->isActive()) {
        // create the midi message:
        MidiProcessor::MidiEvent *event = new MidiProcessor::MidiEvent();
        event->size = 3;
        event->buffer[0] = 0xA0 + channel;
        event->buffer[1] = note;
        event->buffer[2] = pressure;
        getMidiSignalClient()->postEvent(event);
    }
}

void MidiSignalThread::sendControlChange(unsigned char channel, unsigned char controller, unsigned char value)
{
    if (getMidiSignalClient()->isActive()) {
        // create the midi message:
        MidiProcessor::MidiEvent *event = new MidiProcessor::MidiEvent();
        event->size = 3;
        event->buffer[0] = 0xB0 + channel;
        event->buffer[1] = controller;
        event->buffer[2] = value;
        getMidiSignalClient()->postEvent(event);
    }
}

void MidiSignalThread::sendProgramChange(unsigned char channel, unsigned char program)
{
    if (getMidiSignalClient()->isActive()) {
        // create the midi message:
        MidiProcessor::MidiEvent *event = new MidiProcessor::MidiEvent();
        event->size = 2;
        event->buffer[0] = 0xC0 + channel;
        event->buffer[1] = program;
        event->buffer[2] = 0;
        getMidiSignalClient()->postEvent(event);
    }
}

void MidiSignalThread::sendChannelPressure(unsigned char channel, unsigned char pressure)
{
    if (getMidiSignalClient()->isActive()) {
        // create the midi message:
        MidiProcessor::MidiEvent *event = new MidiProcessor::MidiEvent();
        event->size = 2;
        event->buffer[0] = 0xD0 + channel;
        event->buffer[1] = pressure;
        event->buffer[2] = 0;
        getMidiSignalClient()->postEvent(event);
    }
}

void MidiSignalThread::sendPitchWheel(unsigned char channel, unsigned int pitch)
{
    if (getMidiSignalClient()->isActive()) {
        // create the midi message:
        MidiProcessor::MidiEvent *event = new MidiProcessor::MidiEvent();
        event->size = 3;
        event->buffer[0] = 0xE0 + channel;
        event->buffer[1] = pitch & 0x0F;
        event->buffer[2] = pitch >> 7;
        getMidiSignalClient()->postEvent(event);
    }
}

MidiSignalClient::MidiSignalClient(const QString &clientName, size_t ringBufferSize) :
    JackThreadEventProcessorClient(new MidiSignalThread(this), clientName, QStringList(), QStringList(), QStringList("Midi in"), QStringList("Midi out"), ringBufferSize),
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

QGraphicsItem * MidiSignalClient::createGraphicsItem()
{
    return new MidiSignalGraphicsItem(this);
}

bool MidiSignalClient::processEvent(const RingBufferEvent *event, jack_nframes_t time)
{
    if (const MidiProcessor::MidiEvent *midiEvent = dynamic_cast<const MidiProcessor::MidiEvent*>(event)) {
        // write this event to the MIDI output buffer:
        writeMidi(0, *midiEvent, time);
        return true;
    }
    return false;
}

void MidiSignalClient::processMidi(int inputIndex, const MidiProcessor::MidiEvent &event, jack_nframes_t)
{
    // notify the associated thread:
    ringBufferToThread.write(event);
    wakeJackThread();
}

MidiSignalGraphicsItem::MidiSignalGraphicsItem(MidiSignalClient *client, QGraphicsItem *parent) :
    GraphicsKeyboardItem(1, parent)
{
    QObject::connect(this, SIGNAL(keyPressed(unsigned char,unsigned char,unsigned char)), client->getMidiSignalThread(), SLOT(sendNoteOn(unsigned char,unsigned char,unsigned char)));
    QObject::connect(this, SIGNAL(keyReleased(unsigned char,unsigned char,unsigned char)), client->getMidiSignalThread(), SLOT(sendNoteOff(unsigned char,unsigned char,unsigned char)));
}

class MidiSignalClientFactory : public JackClientFactory
{
public:
    MidiSignalClientFactory()
    {
        JackClientSerializer::getInstance()->registerFactory(this);
    }
    QString getName()
    {
        return "Keyboard";
    }
    JackClient * createClient(const QString &clientName)
    {
        return new MidiSignalClient(clientName);
    }
    static MidiSignalClientFactory factory;
};

MidiSignalClientFactory MidiSignalClientFactory::factory;

JackClientFactory * MidiSignalClient::getFactory()
{
    return &MidiSignalClientFactory::factory;
}
