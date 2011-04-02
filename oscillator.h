#ifndef OSCILLATOR_H
#define OSCILLATOR_H

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

class Oscillator : public AudioProcessor, public MidiProcessor, public EventProcessor
{
public:
    class ChangeGainEvent : public RingBufferEvent
    {
    public:
        ChangeGainEvent(double gain_) :
            gain(gain_)
        {}
        double gain;
    };
    class ChangeTuneEvent : public RingBufferEvent
    {
    public:
        ChangeTuneEvent(double tune_) :
            tune(tune_)
        {}
        double tune;
    };
    class ChangePitchModulationIntensityEvent : public RingBufferEvent
    {
    public:
        ChangePitchModulationIntensityEvent(double halfTones_) :
            halfTones(halfTones_)
        {}
        double halfTones;
    };

    Oscillator(double sampleRate = 44100, const QStringList &additionalInputPortNames = QStringList());
    virtual ~Oscillator();

    Oscillator & operator=(const Oscillator &oscillator);

    virtual void save(QDataStream &stream) const;
    virtual void load(QDataStream &stream);

    void setDetuneController(unsigned char controller);
    unsigned char getDetuneController() const;

    // reimplemented from AudioProcessor:
    virtual void setSampleRate(double sampleRate);
    // reimplemented from MidiProcessor:
    virtual void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    virtual void processPitchBend(unsigned char channel, unsigned int value, jack_nframes_t time);
    virtual void processController(unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time);
    // reimplemented from AudioProcessor:
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);
    // reimplemented from EventProcessor:
    virtual bool processEvent(const RingBufferEvent *event, jack_nframes_t time);

    void setGain(double gain);
    double getGain() const;

    /**
      Returns the maximum pitch modulation in half ones
      (12 = +-1 octave maximum pitch modulation).
      */
    double getPitchModulationIntensity() const;
    /**
      Sets the maximum pitch modulation to the value given
      in half tones.
      */
    void setPitchModulationIntensity(double halfTones);

    /**
      Sets the tune (relative to the current frequency)
      in cents (100 cents = 1 half tone, 1200 cents = 1 octave).
      */
    void setTune(double cents);
    /**
      Returns the current tune value in cents.
      */
    double getTune() const;
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
    // persistent member variables:
    unsigned char tuneController;
    double gain, tuneInCents, frequencyModulationIntensity;
    // derived member variables:
    double phase, frequency, frequencyDetuneFactor, frequencyModulationFactorBase, frequencyModulationFactor, frequencyPitchBendFactor, normalizedFrequency;

    void computeNormalizedFrequency();
    void init();
};

#endif // OSCILLATOR_H
