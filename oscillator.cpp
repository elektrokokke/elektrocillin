#include "oscillator.h"
#include <cmath>
#include <QDebug>

Oscillator::Oscillator(double sampleRate) :
    MidiProcessor(0, 1, sampleRate),
    noteNumber(0),
    pitchBendValue(0),
    frequencyInHertz(0.0),
    phase(0.0),
    phaseIncrement(0.0)
{
}

void Oscillator::setSampleRate(double sampleRate)
{
    MidiProcessor::setSampleRate(sampleRate);
    // recompute phase increment:
    computePhaseIncrement();
}

void Oscillator::processNoteOn(unsigned char, unsigned char noteNumber, unsigned char)
{
    this->noteNumber = noteNumber;
    frequencyInHertz = computeFrequencyFromMidiNoteNumber(noteNumber) * computePitchBendFactorFromMidiPitch(pitchBendValue);
    computePhaseIncrement();
}

void Oscillator::processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity)
{
}

void Oscillator::processPitchBend(unsigned char channel, unsigned int value)
{
    pitchBendValue = value;
    frequencyInHertz = computeFrequencyFromMidiNoteNumber(noteNumber) * computePitchBendFactorFromMidiPitch(pitchBendValue);
    computePhaseIncrement();
}

void Oscillator::processAudio(const double *inputs, double *outputs)
{
    outputs[0] = valueAtPhase(phase);
    // advance phase:
    phase += phaseIncrement;
    if (phase >= 2.0 * M_PI) {
        phase -= 2.0 * M_PI;
    }
}

void Oscillator::setFrequency(double hertz)
{
    frequencyInHertz = hertz;
    computePhaseIncrement();
}

double Oscillator::getFrequency() const
{
    return frequencyInHertz;
}

double Oscillator::getPhaseIncrement() const
{
    return phaseIncrement;
}

double Oscillator::valueAtPhase(double phase)
{
    return sin(phase);
}

void Oscillator::computePhaseIncrement()
{
    phaseIncrement = frequencyInHertz * 2.0 * M_PI / getSampleRate();
}

double Oscillator::computeFrequencyFromMidiNoteNumber(unsigned char midiNoteNumber)
{
    // 440 Hz is note number 69:
    double octave = ((double)midiNoteNumber - 69.0) / 12.0;
    double frequency = 440.0 * pow(2.0, octave);
    return frequency;
}

double Oscillator::computePitchBendFactorFromMidiPitch(unsigned int pitchBend)
{
    // center it around 0x2000:
    int pitchCentered = (int)pitchBend - 0x2000;
    // -8192 means minus two half tones => -49152 is one octave => factor 2^(-1)
    // +8192 means plus two half tones => +49152 is one octave => factor 2^1
    return pow(2.0, (double)pitchCentered / 49152.0);
}
