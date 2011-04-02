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

#include "midiprocessor.h"
#include <cmath>

MidiProcessor::~MidiProcessor()
{
}

double MidiProcessor::computeFrequencyFromMidiNoteNumber(unsigned char midiNoteNumber)
{
    // 440 Hz is note number 69:
    double octave = ((double)midiNoteNumber - 69.0) / 12.0;
    double frequency = 440.0 * pow(2.0, octave);
    return frequency;
}

double MidiProcessor::computePitchBendFactorFromMidiPitch(double base, unsigned int pitchBend)
{
    // center it around 0x2000:
    int pitchCentered = (int)pitchBend - 0x2000;
    // -8192 means minus two half tones => -49152 is one octave => factor 2^(-1)
    // +8192 means plus two half tones => +49152 is one octave => factor 2^1
    return pow(base, (double)pitchCentered / 49152.0);
}

void MidiProcessor::processNoteOff(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time)
{
}

void MidiProcessor::processAfterTouch(unsigned char channel, unsigned char noteNumber, unsigned char pressure, jack_nframes_t time)
{
}

void MidiProcessor::processController(unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time)
{
}

void MidiProcessor::processPitchBend(unsigned char channel, unsigned int value, jack_nframes_t time)
{
}

void MidiProcessor::processChannelPressure(unsigned char channel, unsigned char pressure, jack_nframes_t time)
{
}
