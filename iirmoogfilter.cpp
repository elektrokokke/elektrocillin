#include "iirmoogfilter.h"
#include <cmath>

IirMoogFilter::IirMoogFilter(double sampleRate, int zeros) :
    IirFilter(1 + zeros, 4, QStringList("Cutoff modulation"), sampleRate),
    frequencyController(1),
    resonanceController(2)
{
    parameters.frequency = sampleRate * 0.25;
    parameters.frequencyOffsetFactor = 8; // three octaves difference from MIDI input note
    parameters.frequencyPitchBendFactor = 1;
    parameters.frequencyModulationFactor = 1;
    parameters.frequencyModulationIntensity = 1;
    parameters.resonance = 0;
    computeCoefficients();
}

void IirMoogFilter::setFrequencyController(unsigned char controller)
{
    frequencyController = controller;
}

unsigned char IirMoogFilter::getFrequencyController() const
{
    return frequencyController;
}

void IirMoogFilter::setResonanceController(unsigned char controller)
{
    resonanceController = controller;
}

unsigned char IirMoogFilter::getResonanceController() const
{
    return resonanceController;
}

void IirMoogFilter::processAudio(const double *inputs, double *outputs, jack_nframes_t time)
{
    // modify cutoff frequency from second input:
    parameters.frequencyModulationFactor = pow(1 + parameters.frequencyModulationIntensity, inputs[1]);
    computeCoefficients();
    IirFilter::processAudio(inputs, outputs, time);
}

void IirMoogFilter::processNoteOn(unsigned char, unsigned char noteNumber, unsigned char, jack_nframes_t)
{
    // set base cutoff frequency from note number:
    parameters.frequency = parameters.frequencyOffsetFactor * computeFrequencyFromMidiNoteNumber(noteNumber);
    computeCoefficients();
}

void IirMoogFilter::processPitchBend(unsigned char, unsigned int value, jack_nframes_t)
{
    // set cutoff frequency pitch bend factor:
    parameters.frequencyPitchBendFactor = computePitchBendFactorFromMidiPitch(value);
    computeCoefficients();
}

void IirMoogFilter::processController(unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time)
{
    if (controller == resonanceController) {
        // set resonance according to the given controller value:
        parameters.resonance = (double)value / 127.0;
        computeCoefficients();
    } else if (controller == frequencyController) {
        parameters.frequency /= parameters.frequencyOffsetFactor;
        // set cutoff frequency according to the given controller value:
        // 0 means cutoff frequency is one octave above note frequency;
        // 127 means cutoff frequency is 6 octaves above note frequency
        parameters.frequencyOffsetFactor = pow(2.0, (double)value / 127.0 * 5.0 + 1);
        parameters.frequency *= parameters.frequencyOffsetFactor;
        computeCoefficients();
    } else {
        IirFilter::processController(channel, controller, value, time);
    }
}

void IirMoogFilter::setSampleRate(double sampleRate)
{
    IirFilter::setSampleRate(sampleRate);
    // recompute coefficients:
    computeCoefficients();
}

void IirMoogFilter::setParameters(const Parameters parameters)
{
    this->parameters = parameters;
    computeCoefficients();
}

const IirMoogFilter::Parameters & IirMoogFilter::getParameters() const
{
    return parameters;
}

void IirMoogFilter::computeCoefficients()
{
    double cutoffFrequencyInHertz = parameters.frequency * parameters.frequencyPitchBendFactor * parameters.frequencyModulationFactor;
    double radians = convertHertzToRadians(cutoffFrequencyInHertz);
    if (radians > M_PI) {
        radians = M_PI;
    }
    double s = sin(radians);
    double c = cos(radians);
    double t = tan((radians - M_PI) * 0.25);
    double a1 = t / (s - c * t);
    double a2 = a1 * a1;
    double g1Square_inv = 1.0 + a2 + 2.0 * a1 * c;
    double k = parameters.resonance * g1Square_inv * g1Square_inv;
    getFeedBackCoefficients()[0] = k + 4.0 * a1;
    getFeedBackCoefficients()[1] = 6.0 * a2;
    getFeedBackCoefficients()[2] = 4.0 * a2 * a1;
    getFeedBackCoefficients()[3] = a2 * a2;

    int n = getFeedForwardCoefficients().size() - 1;
    double feedBackSum = 1.0 + getFeedBackCoefficients()[0] + getFeedBackCoefficients()[1] + getFeedBackCoefficients()[2] + getFeedBackCoefficients()[3];
    int powerOfTwo = 1 << n;
    double factor = 1.0 / powerOfTwo;
    for (int k = 0; k < (n + 2) / 2; k++) {
        getFeedForwardCoefficients()[k] = factor * IirFilter::computeBinomialCoefficient(n, k) * feedBackSum;
    }
    for (int k = (n + 2) / 2; k <= n; k++) {
        getFeedForwardCoefficients()[k] = getFeedForwardCoefficients()[n - k];
    }
}
