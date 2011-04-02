#ifndef STEPSEQUENCER_H
#define STEPSEQUENCER_H

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

#include "audioprocessor.h"
#include "parameterprocessor.h"

class MidiProcessorClient;

class StepSequencer : public AudioProcessor, public ParameterProcessor
{
public:
    StepSequencer(int nrOfSteps);

    // reimplemented from AudioProcessor:
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);

    // for writing Midi output:
    void setMidiProcessorClient(MidiProcessorClient *midiProcessorClient);
private:
    int nrOfSteps, currentStep;
    double lastBarInput, lastBeatInput;
    MidiProcessorClient *midiProcessorClient;
};

#endif // STEPSEQUENCER_H
