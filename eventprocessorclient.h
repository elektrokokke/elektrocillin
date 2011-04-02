#ifndef EVENTPROCESSORCLIENT_H
#define EVENTPROCESSORCLIENT_H

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

#include "midiprocessorclient.h"
#include "eventprocessor.h"
#include <QVector>

class EventProcessorClient : public MidiProcessorClient
{
public:
    EventProcessorClient(const QString &clientName, AudioProcessor *audioProcessor, MidiProcessor *midiProcessor, EventProcessor *eventProcessor_, size_t ringBufferSize = (2 << 16));

    EventProcessor * getEventProcessor();

    bool postEvent(RingBufferEvent *event);
    bool postEvents(const QVector<RingBufferEvent*> &events);

protected:
    /**
      Constructor for subclasses that do not want to use an EventProcessor,
      but reimplement the respective methods such as to do the EventProcessor's
      work themselves.

      See AudioProcessorClient::AudioProcessorClient(const QString &, const QStringList &, const QStringList &)
      for a parameter description.

      Methods to reimplement:
      - processAudio(const double*, double*, jack_nframes_t) OR
      - processAudio(jack_nframes_t, jack_nframes_t),
      - processNoteOn AND
      - processNoteOff AND
      - processController AND
      - processPitchBend OR
      - processMidi(const MidiEvent &, jack_nframes_t) OR
      - processMidi(jack_nframes_t, jack_nframes_t)
      - processEvent(const RingBufferEvent*, jack_nframes_t)
      */
    EventProcessorClient(const QString &clientName, const QStringList &inputPortNames, const QStringList &outputPortNames, size_t ringBufferSize = (2 << 16));

    // reimplemented from MidiProcessorClient:
    virtual bool process(jack_nframes_t nframes);

    virtual bool processEvents(jack_nframes_t start, jack_nframes_t end, jack_nframes_t nframes);
    /**
      Reimplement this method to process events if you did not provide an EventProcessor object
      at construction, i.e. if you used the second constructor.
      */
    virtual bool processEvent(const RingBufferEvent *event, jack_nframes_t time);

private:
    EventProcessor *eventProcessor;
    RingBuffer ringBuffer;
};

#endif // EVENTPROCESSORCLIENT_H
