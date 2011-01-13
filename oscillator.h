#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include <QStack>

class Oscillator
{
public:
    Oscillator();

    void setSampleRate(double sampleRate);

    void pushNote(unsigned char midiNoteNumber);
    void popNote(unsigned char midiNoteNumber);

    double createSample();

protected:
    virtual double valueAtPhase(double phase);

private:
    QStack<double> frequencies;
    QStack<unsigned char> midiNoteNumbers;
    double sampleRate, phase, phaseIncrement;

    void computePhaseIncrement();
    double computeFrequencyFromMidiNoteNumber(unsigned char midiNoteNumber);
};

#endif // OSCILLATOR_H
