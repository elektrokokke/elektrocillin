#ifndef NOTETRIGGERED_H
#define NOTETRIGGERED_H

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

#include <QStringList>
#include <jack/types.h>
#include "metajack/midiport.h"
#include "jackringbuffer.h"

class MidiProcessor
{
public:
    class MidiEvent : public RingBufferEvent {
    public:
        size_t size;
        jack_midi_data_t buffer[3];
    };
    class MidiWriter
    {
    public:
        virtual void writeMidi(int outputIndex, const MidiEvent &event, jack_nframes_t time) = 0;
    };

    MidiProcessor(const QStringList &midiInputPortNames, const QStringList &midiOutputPortNames, MidiWriter *midiWriter = 0);
    virtual ~MidiProcessor();

    const QStringList & getMidiInputPortNames() const;
    const QStringList & getMidiOutputPortNames() const;
    int getNrOfMidiInputs() const;
    int getNrOfMidiOutputs() const;

    virtual void processNoteOn(int inputIndex, unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    virtual void processNoteOff(int inputIndex, unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    virtual void processAfterTouch(int inputIndex, unsigned char channel, unsigned char noteNumber, unsigned char pressure, jack_nframes_t time);
    virtual void processController(int inputIndex, unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time);
    virtual void processPitchBend(int inputIndex, unsigned char channel, unsigned int value, jack_nframes_t time);
    virtual void processChannelPressure(int inputIndex, unsigned char channel, unsigned char pressure, jack_nframes_t time);

    // these methods allow sending of MIDI events; make sure that you provided a valid MidiWriter object to the constructor,
    // which allows writing of MIDI messages (i.e., which is associated to a MIDI output port), if using these methods!
    void writeMidi(int outputIndex, const MidiEvent &event, jack_nframes_t time);
    void writeNoteOff(int outputIndex, unsigned char channel, unsigned char note, unsigned char velocity, jack_nframes_t time);
    void writeNoteOn(int outputIndex, unsigned char channel, unsigned char note, unsigned char velocity, jack_nframes_t time);
    void writeControlller(int outputIndex, unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time);
    // you may call this as an alternative to providing a MidiWriter in the constructor, before calling any of the above methods:
    void setMidiWriter(MidiWriter *midiWriter);

    // convenience methods:
    static double computeFrequencyFromMidiNoteNumber(unsigned char midiNoteNumber);
    static double computePitchBendFactorFromMidiPitch(double base, unsigned int processPitchBend);
private:
    QStringList midiInputPortNames, midiOutputPortNames;
    MidiWriter *midiWriter;
};

#endif // NOTETRIGGERED_H
