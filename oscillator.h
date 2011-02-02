#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include "midiprocessor.h"

class Oscillator : public MidiProcessor
{
public:
    Oscillator(double sampleRate = 44100);

    virtual void setSampleRate(double sampleRate);
    virtual void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity);
    virtual void processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity);
    virtual void processPitchBend(unsigned char channel, unsigned int value);
    virtual void processAudio(const double *inputs, double *outputs);

    double getPhaseIncrement() const;

protected:
    virtual double valueAtPhase(double phase);

private:
    unsigned char noteNumber;
    int pitchBendValue;
    double frequency, phase, phaseIncrement;

    void computePhaseIncrement();
    static double computeFrequencyFromMidiNoteNumber(unsigned char midiNoteNumber);
    static double computePitchBendFactorFromMidiPitch(unsigned int processPitchBend);
};

#endif // OSCILLATOR_H
