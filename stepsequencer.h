#ifndef STEPSEQUENCER_H
#define STEPSEQUENCER_H

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

#include "audioprocessor.h"
#include "midiparameterprocessor.h"

class StepSequencer : public AudioProcessor, public MidiParameterProcessor
{
public:
    StepSequencer(int nrOfSteps);

    // reimplemented from AudioProcessor:
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);

    void setTransportPosition(jack_transport_state_t state, jack_position_t &position);
private:
    int nrOfSteps, stepsPerBeat, lastStep;
    jack_transport_state_t state;
    jack_position_t position;
    bool noteActive;
    unsigned char channel, noteNumber, velocity;
    int firstParameterIndex;
};

#endif // STEPSEQUENCER_H
