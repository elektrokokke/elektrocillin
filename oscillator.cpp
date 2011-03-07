#include "oscillator.h"
#include <cmath>
#include <QtGlobal>

Oscillator::Oscillator(double frequencyModulationIntensity_, double sampleRate, const QStringList &additionalInputPortNames) :
    MidiProcessor(QStringList("Pitch modulation") + additionalInputPortNames, QStringList("Audio out"), sampleRate),
    detuneController(3),
    gain(1),
    frequency(440),
    frequencyDetuneFactor(1),
    detuneInCents(0),
    frequencyPitchBendFactor(1),
    frequencyModulationFactor(1),
    frequencyModulationIntensity(frequencyModulationIntensity_),
    phase(0)
{
    computeNormalizedFrequency();
}

void Oscillator::setDetuneController(unsigned char controller)
{
    detuneController = controller;
}

unsigned char Oscillator::getDetuneController() const
{
    return detuneController;
}

void Oscillator::setSampleRate(double sampleRate)
{
    MidiProcessor::setSampleRate(sampleRate);
    // recompute phase increment:
    computeNormalizedFrequency();
}

void Oscillator::processNoteOn(unsigned char, unsigned char noteNumber, unsigned char, jack_nframes_t)
{
    frequency = computeFrequencyFromMidiNoteNumber(noteNumber);
    computeNormalizedFrequency();
}

void Oscillator::processPitchBend(unsigned char, unsigned int value, jack_nframes_t)
{
    frequencyPitchBendFactor = computePitchBendFactorFromMidiPitch(value);
    computeNormalizedFrequency();
}

void Oscillator::processController(unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time)
{
    if (controller == detuneController) {
        setDetune(((double)value - 64.0) / 128.0 * 200.0);
    } else {
        MidiProcessor::processController(channel, controller, value, time);
    }
}

void Oscillator::processAudio(const double *inputs, double *outputs, jack_nframes_t)
{
    // consider frequency modulation input:
    frequencyModulationFactor = pow(1 + frequencyModulationIntensity, inputs[0]);
    computeNormalizedFrequency();
    double phase2 = phase + normalizedFrequency;
    if (phase2 >= 1) {
        phase2 -= 1;
    }
    // compute the oscillator output:
    //outputs[0] = valueAtPhase(0.5 * (phase + phase2));
    outputs[0] = gain * valueAtPhase(phase);
    phase = phase2;
}

void Oscillator::setGain(double gain)
{
    this->gain = gain;
}

double Oscillator::getGain() const
{
    return gain;
}

void Oscillator::setFrequency(double hertz)
{
    frequency = hertz;
    computeNormalizedFrequency();
}

double Oscillator::getFrequency() const
{
    return frequency;
}

void Oscillator::setDetune(double cents)
{
    detuneInCents = cents;
    frequencyDetuneFactor = pow(2.0, cents / 1200.0);
    computeNormalizedFrequency();
}

double Oscillator::getDetune() const
{
    return detuneInCents;
}

double Oscillator::getNormalizedFrequency() const
{
    return normalizedFrequency;
}

double Oscillator::valueAtPhase(double phase)
{
    return sin(phase * 2 * M_PI);
}

void Oscillator::computeNormalizedFrequency()
{
    normalizedFrequency = frequency * frequencyDetuneFactor * frequencyPitchBendFactor * frequencyModulationFactor / getSampleRate();
    if (normalizedFrequency <= 0.0) {
        normalizedFrequency = 0.000000000001;
    } else if (normalizedFrequency >= 0.5) {
        normalizedFrequency = 0.5;
    }
}

