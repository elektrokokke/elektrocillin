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

#include "iirmoogfilter.h"
#include <cmath>

IirMoogFilter::IirMoogFilter(double sampleRate, int zeros) :
    IirFilter(1 + zeros, 4, QStringList("Cutoff mod.") + QStringList("Resonance mod."), sampleRate),
    frequencyController(1),
    resonanceController(2),
    recomputeCoefficients(false)
{
    // cutoff frequency in Hertz (default is a quarter the sample rate)
    registerParameter("Base cutoff frequency", sampleRate * 0.25, 0, sampleRate * 0.25, 0);
    // resonance [0:1] (default is zero)
    registerParameter("Resonance", 0, 0, 1, 0);
    // offset between Midi note frequency and resulting cutoff frequency in semitones (default is zero)
    registerParameter("Midi note frequency offset", 0, -36, 36, 1);
    // maximum frequency modulation in semitones (default is one octave)
    registerParameter("Frequency modulation intensity", 12, 1, 36, 1);
    // maximum frequency modulation by pitch bend input in semitones (default is two semitones)
    registerParameter("Pitch bend modulation intensity", 2, 1, 12, 1);
    // maximum frequency modulation by Midi controller in semitones (default is one octave)
    registerParameter("Midi controller modulation intensity", 12, 1, 36, 1);
    // uneditable parameters for cutoff modulation from audio, pitch bend and controller, and for resonance modulation from audio and controller:
    registerParameter("Frequency modulation", 0, 0, 0, 0);
    registerParameter("Cutoff controller", 0, 0, 0, 0);
    registerParameter("Pitch bend", 0, 0, 0, 0);
    registerParameter("Resonance modulation", 0, 0, 0, 0);
    registerParameter("Resonance controller", 0, 0, 0, 0);

    computeCoefficients();
}

IirMoogFilter::IirMoogFilter(const IirMoogFilter &tocopy) :
    IirFilter(tocopy),
    ParameterProcessor(tocopy),
    frequencyController(tocopy.frequencyController),
    resonanceController(tocopy.resonanceController)
{
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
    // cutoff modulation through audio input 2:
    setParameterValue(6, inputs[1]);
    // resonance modulation through audio input 3:
    setParameterValue(9, inputs[2]);
    // compute coefficients if necessary:
    if (recomputeCoefficients) {
        computeCoefficients();
        recomputeCoefficients = false;
    }
    IirFilter::processAudio(inputs, outputs, time);
}

void IirMoogFilter::processNoteOn(unsigned char, unsigned char noteNumber, unsigned char, jack_nframes_t)
{
    // set base cutoff frequency from note number:
    setParameterValue(0, computeFrequencyFromMidiNoteNumber(noteNumber) * pow(2.0, getCutoffMidiNoteOffset() / 12.0));
}

void IirMoogFilter::processPitchBend(unsigned char, unsigned int value, jack_nframes_t)
{
    // cutoff modulation through pitch bend wheel:
    int pitchCentered = (int)value - 0x2000;
    setParameterValue(8, (double)pitchCentered / 8192.0);
}

void IirMoogFilter::processController(unsigned char, unsigned char controller, unsigned char value, jack_nframes_t)
{
    if (controller == resonanceController) {
        // resonance modulation through Midi controller:
        setParameterValue(10, (double)value / 127.0);
    } else if (controller == frequencyController) {
        // cutoff modulation through Midi controller:
        setParameterValue(7, (double)value / 127.0);
    }
}

bool IirMoogFilter::setParameterValue(int index, double value)
{
    if (ParameterProcessor::setParameterValue(index, value)) {
        recomputeCoefficients = true;
        return true;
    } else {
        return false;
    }
}

double IirMoogFilter::getBaseCutoffFrequency() const
{
    return getParameter(0).value;
}

double IirMoogFilter::getResonance() const
{
    return getParameter(1).value;
}

double IirMoogFilter::getCutoffMidiNoteOffset() const
{
    return getParameter(2).value;
}

double IirMoogFilter::getCutoffAudioModulationIntensity() const
{
    return getParameter(3).value;
}

double IirMoogFilter::getCutoffPitchBendModulationIntensity() const
{
    return getParameter(4).value;
}

double IirMoogFilter::getCutoffControllerModulationIntensity() const
{
    return getParameter(5).value;
}

double IirMoogFilter::getCutoffAudioModulation() const
{
    return getParameter(6).value;
}

double IirMoogFilter::getCutoffControllerModulation() const
{
    return getParameter(7).value;
}

double IirMoogFilter::getCutoffPitchBendModulation() const
{
    return getParameter(8).value;
}

double IirMoogFilter::getResonanceAudioModulation() const
{
    return getParameter(9).value;
}

double IirMoogFilter::getResonanceControllerModulation() const
{
    return getParameter(10).value;
}


void IirMoogFilter::setSampleRate(double sampleRate)
{
    IirFilter::setSampleRate(sampleRate);
    recomputeCoefficients = true;
}

void IirMoogFilter::computeCoefficients()
{
    double cutoffPitchBendModulationFactor = pow(2.0, getCutoffPitchBendModulationIntensity() * getCutoffPitchBendModulation() / 12.0);
    double cutoffControllerModulationFactor = pow(2.0, getCutoffControllerModulationIntensity() * getCutoffControllerModulation() / 12.0);
    double cutoffAudioModulationFactor = pow(2.0, getCutoffAudioModulationIntensity() * getCutoffAudioModulation() / 12.0);
    double cutoffFrequencyInHertz = getBaseCutoffFrequency() * cutoffPitchBendModulationFactor * cutoffControllerModulationFactor * cutoffAudioModulationFactor;
    double resonance = getResonance() + getResonanceAudioModulation() + getResonanceControllerModulation();
    if (resonance < -1.0) {
        resonance += 2.0;
    } else if (resonance < 0.0) {
        resonance = -resonance;
    } else if (resonance > 2.0) {
        resonance -= 2.0;
    } else if (resonance > 1.0) {
        resonance = 2.0 - resonance;
    }

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
    double k = resonance * g1Square_inv * g1Square_inv;
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
