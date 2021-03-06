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

#include "stepsequencer.h"

StepSequencer::StepSequencer(int nrOfSteps_) :
    AudioProcessor(QStringList("Bar"), QStringList()),
    MidiParameterProcessor(QStringList(), QStringList("Midi note out")),
    nrOfSteps(nrOfSteps_),
    stepsPerBeat(4),
    lastStep(-1),
    state(JackTransportStopped),
    noteActive(false),
    channel(0),
    noteNumber(50),
    velocity(127)
{
    firstParameterIndex = getNrOfParameters();
    for (int i = 0; i < nrOfSteps; i++) {
        registerParameter(QString("Active %1?").arg(i), 0, 0, 1, 1);
    }
}

void StepSequencer::processAudio(const double *inputs, double *, jack_nframes_t time)
{
    if ((state != JackTransportRolling) && noteActive) {
        writeNoteOff(1, channel, noteNumber, velocity, time);
        noteActive = false;
    }
    if (state == JackTransportRolling) {
        // determine wether a new beat has started:
        double beat = 0.5 * (inputs[0] + 1.0) * position.beats_per_bar;
        int step = (int)(beat * stepsPerBeat);
        if (step != lastStep) {
            // deactivate any active note:
            if (noteActive) {
                writeNoteOff(1, channel, noteNumber, velocity, time);
                noteActive = false;
            }
            if ((step < nrOfSteps) && getParameter(firstParameterIndex + step).value) {
                // activate the new note:
                writeNoteOn(1, channel, noteNumber, velocity, time);
                noteActive = true;
            }
        }
        lastStep = step;
    }
}

void StepSequencer::setTransportPosition(jack_transport_state_t state, jack_position_t &position)
{
    this->state = state;
    this->position = position;
}
