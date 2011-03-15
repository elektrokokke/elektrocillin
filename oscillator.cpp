#include "oscillator.h"
#include <cmath>
#include <QtGlobal>

Oscillator::Oscillator(double sampleRate, const QStringList &additionalInputPortNames) :
    AudioProcessor(QStringList("Pitch modulation") + additionalInputPortNames, QStringList("Audio out"), sampleRate),
    tuneController(3),
    gain(1),
    frequency(440),
    frequencyDetuneFactor(1),
    tuneInCents(0),
    frequencyPitchBendFactor(1),
    frequencyModulationFactor(1),
    frequencyModulationIntensity(2),
    phase(0)
{
}

Oscillator::~Oscillator()
{
}

void Oscillator::setDetuneController(unsigned char controller)
{
    tuneController = controller;
}

unsigned char Oscillator::getDetuneController() const
{
    return tuneController;
}

void Oscillator::setSampleRate(double sampleRate)
{
    AudioProcessor::setSampleRate(sampleRate);
}

void Oscillator::processNoteOn(unsigned char, unsigned char noteNumber, unsigned char, jack_nframes_t)
{
    frequency = computeFrequencyFromMidiNoteNumber(noteNumber);
}

void Oscillator::processPitchBend(unsigned char, unsigned int value, jack_nframes_t)
{
    frequencyPitchBendFactor = computePitchBendFactorFromMidiPitch(value);
}

void Oscillator::processController(unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time)
{
    if (controller == tuneController) {
        setTune(((double)value - 64.0) / 128.0 * 200.0);
    } else {
        MidiProcessor::processController(channel, controller, value, time);
    }
}

void Oscillator::processAudio(const double *inputs, double *outputs, jack_nframes_t)
{
    // consider frequency modulation input:
    frequencyModulationFactor = pow(1 + frequencyModulationIntensity / 12.0, inputs[0]);
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

bool Oscillator::processEvent(const RingBufferEvent *event, jack_nframes_t)
{
    if (const ChangeGainEvent *event_ = dynamic_cast<const ChangeGainEvent*>(event)) {
        setGain(event_->gain);
        return true;
    } else if (const ChangeTuneEvent *event_ = dynamic_cast<const ChangeTuneEvent*>(event)) {
        setTune(event_->tune);
        return true;
    } else if (const ChangePitchModulationIntensityEvent *event_ = dynamic_cast<const ChangePitchModulationIntensityEvent*>(event)) {
        setPitchModulationIntensity(event_->halfTones);
        return true;
    }
    return false;
}

void Oscillator::setGain(double gain)
{
    this->gain = gain;
}

double Oscillator::getGain() const
{
    return gain;
}

double Oscillator::getPitchModulationIntensity() const
{
    return frequencyModulationIntensity;
}

void Oscillator::setPitchModulationIntensity(double halfTones)
{
    frequencyModulationIntensity = halfTones;
}

void Oscillator::setFrequency(double hertz)
{
    frequency = hertz;
}

double Oscillator::getFrequency() const
{
    return frequency;
}

void Oscillator::setTune(double cents)
{
    tuneInCents = cents;
    frequencyDetuneFactor = pow(2.0, cents / 1200.0);
}

double Oscillator::getTune() const
{
    return tuneInCents;
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

