/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

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

IirMoogFilter::IirMoogFilter(int zeros) :
    IirFilter(1 + zeros, 4, QStringList("Cutoff mod.") + QStringList("Resonance mod.")),
    recomputeCoefficients(false)
{
    // cutoff frequency in Hertz (default is a quarter the sample rate)
    registerParameter("Base cutoff frequency", 440, 0, 0, 0);
    // resonance [0:1] (default is zero)
    registerParameter("Resonance", 0, 0, 1, 0);
    // offset between Midi note frequency and resulting cutoff frequency in semitones (default is zero)
    registerParameter("Midi note frequency offset", 0, -36, 36, 1);
    // maximum frequency modulation in semitones (default is one octave)
    registerParameter("Frequency modulation intensity", 12, 1, 36, 1);
    // maximum frequency modulation by pitch bend input in semitones (default is two semitones)
    registerParameter("Pitch bend modulation intensity", 2, 0, 12, 1);
    // maximum frequency modulation by Midi controller in semitones (default is one octave)
    registerParameter("Midi controller modulation intensity", 12, 1, 36, 1);
    registerParameter("Cutoff controller", 0, -1, 1, 0);
    // uneditable parameters for cutoff modulation from audio, pitch bend and controller, and for resonance modulation from audio and controller:
    registerParameter("Frequency modulation", 0, 0, 0, 0);
    registerParameter("Resonance modulation", 0, 0, 0, 0);
    registerParameter("Pitch bend", 0, 0, 0, 0);

    computeCoefficients();
}

IirMoogFilter::IirMoogFilter(const IirMoogFilter &tocopy) :
    IirFilter(tocopy),
    MidiParameterProcessor(tocopy)
{
}

void IirMoogFilter::setSampleRate(double sampleRate)
{
    IirFilter::setSampleRate(sampleRate);
    recomputeCoefficients = true;
    // adapt the maximum cutoff frequency:
    getParameter(1).max = 0.5 * sampleRate;
}

void IirMoogFilter::processAudio(const double *inputs, double *outputs, jack_nframes_t time)
{
    // cutoff modulation through audio input 2:
    ParameterProcessor::setParameterValue(8, inputs[1], time);
    // resonance modulation through audio input 3:
    ParameterProcessor::setParameterValue(9, inputs[2], time);
    // compute coefficients if necessary:
    computeCoefficients();
    IirFilter::processAudio(inputs, outputs, time);
}

void IirMoogFilter::processNoteOn(unsigned char, unsigned char noteNumber, unsigned char, jack_nframes_t time)
{
    // set base cutoff frequency from note number:
    ParameterProcessor::setParameterValue(1, computeFrequencyFromMidiNoteNumber(noteNumber) * pow(2.0, getCutoffMidiNoteOffset() / 12.0), time);
}

void IirMoogFilter::processPitchBend(unsigned char, unsigned int value, jack_nframes_t time)
{
    // cutoff modulation through pitch bend wheel:
    int pitchCentered = (int)value - 0x2000;
    ParameterProcessor::setParameterValue(10, (double)pitchCentered / 8192.0, time);
}

bool IirMoogFilter::setParameterValue(int index, double value, double min, double max, unsigned int time)
{
    if (MidiParameterProcessor::setParameterValue(index, value, min, max, time)) {
        recomputeCoefficients = true;
        return true;
    } else {
        return false;
    }
}

double IirMoogFilter::getBaseCutoffFrequency() const
{
    return getParameter(1).value;
}

double IirMoogFilter::getResonance() const
{
    return getParameter(2).value;
}

double IirMoogFilter::getCutoffMidiNoteOffset() const
{
    return getParameter(3).value;
}

double IirMoogFilter::getCutoffAudioModulationIntensity() const
{
    return getParameter(4).value;
}

double IirMoogFilter::getCutoffPitchBendModulationIntensity() const
{
    return getParameter(5).value;
}

double IirMoogFilter::getCutoffControllerModulationIntensity() const
{
    return getParameter(6).value;
}

double IirMoogFilter::getCutoffControllerModulation() const
{
    return getParameter(7).value;
}

double IirMoogFilter::getCutoffAudioModulation() const
{
    return getParameter(8).value;
}

double IirMoogFilter::getResonanceAudioModulation() const
{
    return getParameter(9).value;
}

double IirMoogFilter::getCutoffPitchBendModulation() const
{
    return getParameter(10).value;
}

bool IirMoogFilter::computeCoefficients()
{
    if (recomputeCoefficients) {
        double cutoffFrequencyInHertz = getBaseCutoffFrequency() * pow(2.0, (getCutoffPitchBendModulationIntensity() * getCutoffPitchBendModulation() +  getCutoffControllerModulationIntensity() * getCutoffControllerModulation() + getCutoffAudioModulationIntensity() * getCutoffAudioModulation()) / 12.0);
        double resonance = getResonance() + getResonanceAudioModulation();
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
        recomputeCoefficients = false;
        return true;
    } else {
        return false;
    }
}
