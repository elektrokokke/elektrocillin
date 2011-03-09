#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include "eventprocessor.h"

class Oscillator : public EventProcessor
{
public:
    class ChangeGainEvent : public RingBufferEvent
    {
    public:
        double gain;
    };

    Oscillator(double frequencyModulationIntensity = 2.0/12.0, double sampleRate = 44100, const QStringList &additionalInputPortNames = QStringList());

    void setDetuneController(unsigned char controller);
    unsigned char getDetuneController() const;

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
      Sets the current oscillator frequency
      to the given value in Hertz.
      */
    void setFrequency(double hertz);
    /**
      Returns the current oscillator frequency in Hertz.
      */
    double getFrequency() const;
    /**
      Sets the detune (relative to the current frequency)
      in cents (100 cents = 1 half tone, 1200 cents = 1 octave).
      */
    void setDetune(double cents);
    /**
      Returns the current detune value in cents.
      */
    double getDetune() const;
    /**
      Returns the current normalized frequency of this oscillator,
      i.e. the number of cycles per sample. This number depends on
      the current sample rate (given in Hertz) and the current
      oscillator frequency (in Hertz).

      The maximum normalized frequency is 0.5 (the Nyquist frequency).
      */
    double getNormalizedFrequency() const;

protected:
    virtual double valueAtPhase(double phase);

private:
    unsigned char detuneController;
    double gain, frequency, frequencyDetuneFactor, detuneInCents, frequencyPitchBendFactor, frequencyModulationFactor, frequencyModulationIntensity, phase, normalizedFrequency;

    void computeNormalizedFrequency();
};

#endif // OSCILLATOR_H
