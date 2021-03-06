#ifndef IIRMOOGFILTER_H
#define IIRMOOGFILTER_H

/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Elektrocillin is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Elektrocillin.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "iirfilter.h"
#include "eventprocessor.h"
#include "midiparameterprocessor.h"
#include <QVector>

class IirMoogFilter : public IirFilter, public MidiParameterProcessor
{
public:
    IirMoogFilter(int zeros = 0);
    IirMoogFilter(const IirMoogFilter &tocopy);

    virtual void setSampleRate(double sampleRate);

    // reimplemented from IirFilter (originally from AudioProcessor):
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);
    // reimplemented from MidiProcessor:
    virtual void processNoteOn(int inputIndex, unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    virtual void processPitchBend(int inputIndex, unsigned char channel, unsigned int value, jack_nframes_t time);
    // reimplemented from MidiParameterProcessor:
    virtual bool setParameterValue(int index, double value, double min, double max, unsigned int time);

    double getBaseCutoffFrequency() const;
    double getResonance() const;
    double getCutoffMidiNoteOffset() const;
    double getCutoffAudioModulationIntensity() const;
    double getCutoffPitchBendModulationIntensity() const;
    double getCutoffControllerModulationIntensity() const;
    double getCutoffAudioModulation() const;
    double getCutoffControllerModulation() const;
    double getCutoffPitchBendModulation() const;
    double getResonanceAudioModulation() const;

    bool computeCoefficients();
private:
    bool recomputeCoefficients;
};

#endif // IIRMOOGFILTER_H
