#ifndef MIDICLIENT_H
#define MIDICLIENT_H

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
#include "jackthreadeventprocessorclient.h"
#include "graphicskeyboarditem.h"

class MidiSignalClient;

class MidiSignalThread : public JackThread {
    Q_OBJECT
public:
    MidiSignalThread(MidiSignalClient *client, QObject *parent = 0);

    MidiSignalClient *getMidiSignalClient();

    void setRingBufferFromClient(JackRingBuffer<MidiProcessor::MidiEvent> *ringBufferFromClient);
signals:
    void receivedNoteOff(unsigned char channel, unsigned char note, unsigned char velocity);
    void receivedNoteOn(unsigned char channel, unsigned char note, unsigned char velocity);
    void receivedAfterTouch(unsigned char channel, unsigned char note, unsigned char pressure);
    void receivedControlChange(unsigned char channel, unsigned char controller, unsigned char value);
    void receivedProgramChange(unsigned char channel, unsigned char program);
    void receivedChannelPressure(unsigned char channel, unsigned char pressure);
    void receivedPitchWheel(unsigned char channel, unsigned int pitch);
public slots:
    void sendNoteOff(unsigned char channel, unsigned char note, unsigned char velocity);
    void sendNoteOn(unsigned char channel, unsigned char note, unsigned char velocity);
    void sendAfterTouch(unsigned char channel, unsigned char note, unsigned char pressure);
    void sendControlChange(unsigned char channel, unsigned char controller, unsigned char value);
    void sendProgramChange(unsigned char channel, unsigned char program);
    void sendChannelPressure(unsigned char channel, unsigned char pressure);
    void sendPitchWheel(unsigned char channel, unsigned int pitch);
protected:
    void processDeferred();
private:
    // This is still the template class because the other class involves new and delete, which should not be in the Jack process thread (which is the sending thread here)
    JackRingBuffer<MidiProcessor::MidiEvent> *ringBufferFromClient;
};

class MidiSignalClient : public JackThreadEventProcessorClient {
public:
    MidiSignalClient(const QString &clientName, size_t ringBufferSize = 1024);
    virtual ~MidiSignalClient();

    virtual JackClientFactory * getFactory();

    MidiSignalThread * getMidiSignalThread();

    QGraphicsItem * createGraphicsItem();

protected:
    // reimplemented from EventProcessorClient2:
    virtual bool processEvent(const RingBufferEvent *event, jack_nframes_t time);
    // reimplemented from MidiProcessor:
    virtual void processMidi(const MidiProcessor::MidiEvent &event, jack_nframes_t time);

private:
    // This is still the template class because the other class involves new and delete, which should not be in the Jack process thread (which is the sending thread here)
    JackRingBuffer<MidiProcessor::MidiEvent> ringBufferToThread;
};

class MidiSignalGraphicsItem : public GraphicsKeyboardItem
{
    Q_OBJECT
public:
    MidiSignalGraphicsItem(MidiSignalClient *client, QGraphicsItem *parent = 0);
};

struct MidiEventWithTimeStamp {
    jack_nframes_t time;
    MidiProcessor::MidiEvent event;
};

#endif // MIDICLIENT_H
