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

#include "oscillator.h"
#include <cmath>
#include <QtGlobal>

Oscillator::Oscillator(const QStringList &additionalInputPortNames, MidiProcessor::MidiWriter *midiWriter) :
    AudioProcessor(QStringList("Pitch modulation") + additionalInputPortNames, QStringList("Audio out")),
    MidiParameterProcessor(QStringList("Midi note in"), QStringList(), midiWriter),
    pitchBend(0),
    pitchModulation(0)
{
    registerParameter("Gain", 1, 0, 1, 0.01);
    registerParameter("Octave", 0, -3, 3, 1);
    registerParameter("Tune (cents)", 0, -100, 100, 1);
    registerParameter("Pitch mod. intensity", 12, 0, 24, 1);
    // maximum frequency modulation by pitch bend input in semitones (default is two semitones)
    registerParameter("Pitch bend modulation intensity", 2, 0, 12, 1);
    init();
}

Oscillator::~Oscillator()
{
}

Oscillator & Oscillator::operator=(const Oscillator &oscillator)
{
    // copy all relevant attributes of the given oscillator:
    ParameterProcessor::operator =(oscillator);
    init();
    return *this;
}

void Oscillator::processNoteOn(int inputIndex, unsigned char, unsigned char noteNumber, unsigned char, jack_nframes_t)
{
    if (inputIndex == 1) {
        frequency = computeFrequencyFromMidiNoteNumber(noteNumber);
    }
}

void Oscillator::processPitchBend(int inputIndex, unsigned char, unsigned int value, jack_nframes_t time)
{
    if (inputIndex == 1) {
        int pitchCentered = (int)value - 0x2000;
        pitchBend = (double)pitchCentered / 8192.0;
    }
}

void Oscillator::processAudio(const double *inputs, double *outputs, jack_nframes_t time)
{
    // consider frequency modulation input:
    pitchModulation = inputs[0];
    computeNormalizedFrequency();
    double phase2 = phase + normalizedFrequency;
    if (phase2 >= 1) {
        phase2 -= 1;
    }
    // compute the oscillator output:
    outputs[0] = getParameter(1).value * valueAtPhase(phase);
    phase = phase2;
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
    double octave = getParameter(2).value;
    double tune = getParameter(3).value;
    double pitchModulationIntensity = getParameter(4).value;
    double pitchBendIntensity = getParameter(5).value;
    normalizedFrequency = frequency * pow(2.0, octave + pitchModulation * pitchModulationIntensity / 12.0 + tune / 1200.0 + pitchBend * pitchBendIntensity / 12.0) / getSampleRate();
    if (normalizedFrequency < 0.0) {
        normalizedFrequency = 0;
    } else if (normalizedFrequency > 0.5) {
        normalizedFrequency = 0.5;
    }
}

void Oscillator::init()
{
    // set derived attributes:
    phase = 0;
    frequency = 440;
    computeNormalizedFrequency();
}
