#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include "notetriggered.h"

class Oscillator : public NoteTriggered
{
public:
    Oscillator(double sampleRate = 44100);

    virtual void setSampleRate(double sampleRate);
    virtual void noteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity);
    virtual void noteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity);
    virtual void pitchBend(unsigned char channel, unsigned int value);
    virtual void process(const double *inputs, double *outputs);

    double getPhaseIncrement() const;

protected:
    virtual double valueAtPhase(double phase);

private:
    unsigned char noteNumber;
    int pitchBendValue;
    double frequency, phase, phaseIncrement;

    void computePhaseIncrement();
    static double computeFrequencyFromMidiNoteNumber(unsigned char midiNoteNumber);
    static double computePitchBendFactorFromMidiPitch(unsigned int pitchBend);
};

#endif // OSCILLATOR_H
