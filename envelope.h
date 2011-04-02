#ifndef ENVELOPE_H
#define ENVELOPE_H

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

#include "audioprocessor.h"
#include "midiprocessor.h"
#include "eventprocessor.h"
#include "interpolatorprocessor.h"
#include "linearinterpolator.h"
#include "logarithmicinterpolator.h"

class Envelope : public AudioProcessor, public MidiProcessor, public EventProcessor, public InterpolatorProcessor
{
public:
    enum Phase {
        ATTACK = 0,
        RELEASE = 1,
        NONE,
        SUSTAIN
    };
    class ChangeSustainPositionEvent : public RingBufferEvent
    {
    public:
        ChangeSustainPositionEvent(int sustainIndex_) :
            sustainIndex(sustainIndex_)
        {}
        int sustainIndex;
    };

    Envelope(double durationInSeconds = 20, double sampleRate = 44100);

    Envelope & operator=(const Envelope &envelope);

    void save(QDataStream &stream) const;
    void load(QDataStream &stream);

    Interpolator * getInterpolator();
    void copyInterpolatorFrom(const Envelope *envelope);

    int getSustainIndex() const;
    void setSustainIndex(int sustainIndex);

    double getDurationInSeconds() const;

    // reimplemented from MidiProcessor:
    virtual void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    virtual void processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    // reimplemented from AudioProcessor:
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);
    // reimplemented from EventProcessor:
    virtual bool processEvent(const RingBufferEvent *event, jack_nframes_t time);
private:
    double durationInSeconds;
    double currentTime, previousLevel, velocity;
    int sustainIndex;
    Phase currentPhase;
    bool release;
    double startLevel;
    //LinearInterpolator interpolator;
    LogarithmicInterpolator interpolator;
};

#endif // ENVELOPE_H
