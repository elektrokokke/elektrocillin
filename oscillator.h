#ifndef OSCILLATOR_H
#define OSCILLATOR_H

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

#include "audioprocessor.h"
#include "midiprocessor.h"
#include "eventprocessor.h"
#include "parameterprocessor.h"

class Oscillator : public AudioProcessor, public MidiProcessor, public ParameterProcessor
{
public:
    Oscillator(const QStringList &additionalInputPortNames = QStringList());
    virtual ~Oscillator();

    Oscillator & operator=(const Oscillator &oscillator);

    // reimplemented from AudioProcessor:
    virtual void setSampleRate(double sampleRate);
    // reimplemented from MidiProcessor:
    virtual void processNoteOn(int inputIndex, unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    virtual void processPitchBend(int inputIndex, unsigned char channel, unsigned int value, jack_nframes_t time);
    virtual void processController(int inputIndex, unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time);
    // reimplemented from AudioProcessor:
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);

protected:
    /**
      Returns the current normalized frequency of this oscillator,
      i.e. the number of cycles per sample. This number depends on
      the current sample rate (given in Hertz) and the current
      oscillator frequency (in Hertz).

      The maximum normalized frequency is 0.5 (the Nyquist frequency).
      */
    double getNormalizedFrequency() const;
    virtual double valueAtPhase(double phase);

private:
    // derived member variables:
    double phase, frequency, normalizedFrequency;

    void computeNormalizedFrequency();
    void init();
};

#endif // OSCILLATOR_H
