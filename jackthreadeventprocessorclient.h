#ifndef JACKTHREADEVENTPROCESSOR_H
#define JACKTHREADEVENTPROCESSOR_H

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

#include "eventprocessorclient.h"
#include "jackthread.h"

class JackThreadEventProcessorClient : public EventProcessorClient
{
public:
    JackThreadEventProcessorClient(JackThread *thread, const QString &clientName, AudioProcessor *audioProcessor, MidiProcessor *midiProcessor, EventProcessor *eventProcessor, size_t ringBufferSize = 1024) :
        EventProcessorClient(clientName, audioProcessor, midiProcessor, eventProcessor, ringBufferSize),
        jackThread(thread)
    {}
    /**
      Constructor for subclasses that do not want to use a MidiProcessor,
      but reimplement the respective methods such as to do the MidiProcessor's
      work themselves.
      Methods to reimplement:
      - processAudio(const double*, double*, jack_nframes_t) OR
      - processAudio(jack_nframes_t, jack_nframes_t),
      - processNoteOn AND
      - processNoteOff AND
      - processController AND
      - processPitchBend OR
      - processMidi(const MidiEvent &, jack_nframes_t) OR
      - processMidi(jack_nframes_t, jack_nframes_t)
      */
    JackThreadEventProcessorClient(JackThread *thread, const QString &clientName, const QStringList &inputPortNames, const QStringList &outputPortNames, size_t ringBufferSize = 1024) :
        EventProcessorClient(clientName, inputPortNames, outputPortNames, ringBufferSize),
        jackThread(thread)
    {}

    JackThread * getJackThread()
    {
        return jackThread;
    }
    void wakeJackThread()
    {
        jackThread->wake();
    }

protected:
    /**
      This starts the associated JackThread.
      You must call this in your reimplementation, but only AFTER initializing any ring buffers
      the JackThread might use (you could of course also initialize ring buffers in the
      constructor instead).
      */
    virtual bool init()
    {
        jackThread->start();
        return EventProcessorClient::init();
    }
    /**
      This stops the associated JackThread.
      You must call this in your reimplementation, but only BEFORE freeing any ring buffers
      the JackThread might use.
      */
    virtual void deinit()
    {
        jackThread->stop();
        EventProcessorClient::deinit();
    }

private:
    JackThread *jackThread;

};

#endif // JACKTHREADEVENTPROCESSOR_H
