#ifndef NOTETRIGGEREDCLIENT_H
#define NOTETRIGGEREDCLIENT_H

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

#include "audioprocessorclient.h"
#include "midiprocessor.h"

class MidiProcessorClient : public AudioProcessorClient, public MidiProcessor::MidiWriter
{
public:
    struct MidiInputPort {
        jack_port_t *port;
        void *buffer;
        jack_nframes_t eventIndex;
        jack_nframes_t eventCount;
        MidiProcessor::MidiEvent event;
        jack_nframes_t time;
    };
    struct MidiOutputPort {
        jack_port_t *port;
        void *buffer;
    };

    MidiProcessorClient(const QString &clientName, AudioProcessor *audioProcessor, MidiProcessor *midiProcessor, unsigned int channelMask = (1 << 16) - 1);
    virtual ~MidiProcessorClient();

    MidiProcessor * getMidiProcessor();

    void setChannelMask(unsigned int channelMask = (1 << 16) - 1);
    unsigned int getChannelMask() const;

    /**
      This may only be called from any of the process...() methods!
      */
    virtual void writeMidi(int outputIndex, const MidiProcessor::MidiEvent &event, jack_nframes_t time);

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
    MidiProcessorClient(const QString &clientName, const QStringList &audioInputPortNames, const QStringList &audioOutputPortNames, const QStringList &midiInputPortNames, const QStringList &midiOutputPortNames, unsigned int channels = (1 << 16) - 1);

    // reimplemented methods from AudioProcessorClient:
    virtual bool init();
    virtual bool process(jack_nframes_t nframes);

    virtual void processMidi(jack_nframes_t start, jack_nframes_t end);
    virtual void processMidi(int inputIndex, const MidiProcessor::MidiEvent &event, jack_nframes_t time);

    /**
      Override this in subclasses if you did not provide a MidiProcessor in the constructor.
      */
    virtual void processNoteOn(int inputIndex, unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    /**
      Override this in subclasses if you did not provide a MidiProcessor in the constructor.
      */
    virtual void processNoteOff(int inputIndex, unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    /**
      Override this in subclasses if you did not provide a MidiProcessor in the constructor.
      */
    virtual void processAfterTouch(int inputIndex, unsigned char channel, unsigned char noteNumber, unsigned char pressure, jack_nframes_t time);
    /**
      Override this in subclasses if you did not provide a MidiProcessor in the constructor.
      */
    virtual void processController(int inputIndex, unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time);
    /**
      Override this in subclasses if you did not provide a MidiProcessor in the constructor.
      */
    virtual void processPitchBend(int inputIndex, unsigned char channel, unsigned int value, jack_nframes_t time);
    /**
      Override this in subclasses if you did not provide a MidiProcessor in the constructor.
      */
    virtual void processChannelPressure(int inputIndex, unsigned char channel, unsigned char pressure, jack_nframes_t time);

    /**
      If you override process(jack_nframes_t nframes) call this function
      before you call processMidi(jack_nframes_t start, jack_nframes_t end).
      */
    void getMidiPortBuffers(jack_nframes_t nframes);

private:
    MidiProcessor *midiProcessor;
    jack_nframes_t nframes;
    unsigned int channels;
    QStringList midiInputPortNames, midiOutputPortNames;
    QVector<MidiInputPort> midiInputPorts;
    QVector<MidiOutputPort> midiOutputPorts;
};

#endif // NOTETRIGGEREDCLIENT_H
