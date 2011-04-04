#ifndef NOTETRIGGEREDCLIENT_H
#define NOTETRIGGEREDCLIENT_H

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

#include "audioprocessorclient.h"
#include "midiprocessor.h"
#include "metajack/midiport.h"
#include "jackringbuffer.h"

class MidiProcessorClient : public AudioProcessorClient
{
public:
    class MidiEvent : public RingBufferEvent {
    public:
        size_t size;
        jack_midi_data_t buffer[3];
    };

    MidiProcessorClient(const QString &clientName, AudioProcessor *audioProcessor, MidiProcessor *midiProcessor, unsigned int channelMask = (1 << 16) - 1);
    virtual ~MidiProcessorClient();

    MidiProcessor * getMidiProcessor();

    void setChannelMask(unsigned int channelMask = (1 << 16) - 1);
    unsigned int getChannelMask() const;

    /**
      This may only be called from any of the process...() methods!
      */
    void writeMidi(const MidiEvent &event, jack_nframes_t time);
    void writeNoteOff(unsigned char channel, unsigned char note, unsigned char velocity, jack_nframes_t time);
    void writeNoteOn(unsigned char channel, unsigned char note, unsigned char velocity, jack_nframes_t time);

protected:
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

      See AudioProcessorClient::AudioProcessorClient(const QString &, const QStringList &, const QStringList &)
      for a description of the parameters.
      */
    MidiProcessorClient(const QString &clientName, const QStringList &inputPortNames, const QStringList &outputPortNames, unsigned int channels = (1 << 16) - 1);

    void activateMidiInput(bool active);
    void activateMidiOutput(bool active);

    // reimplemented methods from AudioProcessorClient:
    virtual bool init();
    virtual bool process(jack_nframes_t nframes);

    virtual void processMidi(jack_nframes_t start, jack_nframes_t end);
    virtual void processMidi(const MidiEvent &event, jack_nframes_t time);

    /**
      Override this in subclasses if you did not provide a MidiProcessor in the constructor.
      */
    virtual void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    /**
      Override this in subclasses if you did not provide a MidiProcessor in the constructor.
      */
    virtual void processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    /**
      Override this in subclasses if you did not provide a MidiProcessor in the constructor.
      */
    virtual void processAfterTouch(unsigned char channel, unsigned char noteNumber, unsigned char pressure, jack_nframes_t time);
    /**
      Override this in subclasses if you did not provide a MidiProcessor in the constructor.
      */
    virtual void processController(unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time);
    /**
      Override this in subclasses if you did not provide a MidiProcessor in the constructor.
      */
    virtual void processPitchBend(unsigned char channel, unsigned int value, jack_nframes_t time);
    /**
      Override this in subclasses if you did not provide a MidiProcessor in the constructor.
      */
    virtual void processChannelPressure(unsigned char channel, unsigned char pressure, jack_nframes_t time);

    /**
      If you override process(jack_nframes_t nframes) call this function
      before you call processMidi(jack_nframes_t start, jack_nframes_t end).
      */
    void getMidiPortBuffer(jack_nframes_t nframes);

private:
    MidiProcessor *midiProcessor;
    jack_port_t *midiInputPort, *midiOutputPort;
    void *midiInputBuffer, *midiOutputBuffer;
    jack_nframes_t currentMidiEventIndex;
    jack_nframes_t midiEventCount;
    jack_nframes_t nframes;
    bool midiInput, midiOutput;
    unsigned int channels;
};

#endif // NOTETRIGGEREDCLIENT_H
