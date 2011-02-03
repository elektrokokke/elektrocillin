#include "iirmoogfilter.h"
#include <cmath>

IirMoogFilter::IirMoogFilter(double sampleRate, int zeros) :
    IirFilter(1 + zeros, 4, 2, sampleRate)
{
    parameters.frequency = sampleRate * 0.5;
    parameters.frequencyOffsetFactor = 1;
    parameters.frequencyPitchBendFactor = 1;
    parameters.frequencyModulationFactor = 1;
    parameters.frequencyModulationIntensity = 1;
    parameters.resonance = 0;
}

void IirMoogFilter::processAudio(const double *inputs, double *outputs)
{
    // modify cutoff frequency from second input:
    parameters.frequencyModulationFactor = pow(1 + parameters.frequencyModulationIntensity, inputs[1]);
    computeCoefficients();
    IirFilter::processAudio(inputs, outputs);
}

void IirMoogFilter::processNoteOn(unsigned char, unsigned char noteNumber, unsigned char)
{
    // set base cutoff frequency from note number:
    parameters.frequency = computeFrequencyFromMidiNoteNumber(noteNumber);
    computeCoefficients();
}

void IirMoogFilter::processPitchBend(unsigned char, unsigned int value)
{
    // set cutoff frequency pitch bend factor:
    parameters.frequencyPitchBendFactor = computePitchBendFactorFromMidiPitch(value);
    computeCoefficients();
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
    double cutoffFrequencyInHertz = parameters.frequency * parameters.frequencyOffsetFactor * parameters.frequencyPitchBendFactor * parameters.frequencyModulationFactor;
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
