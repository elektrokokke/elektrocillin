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

#include "stepsequencer.h"
#include "midiprocessorclient.h"

StepSequencer::StepSequencer(int nrOfSteps_) :
    AudioProcessor(QStringList("Bar") + QStringList("Beat"), QStringList()),
    nrOfSteps(nrOfSteps_),
    currentStep(0),
    lastBarInput(-1),
    lastBeatInput(-1)
{
    for (int i = 0; i < nrOfSteps; i++) {
        registerParameter(QString("Active %1?").arg(i), 0, 0, 1, 1);
    }
}

void StepSequencer::processAudio(const double *inputs, double *outputs, jack_nframes_t time)
{
//    // monitor bar input to see wether to reset current step:
//    if (lastBarInput > inputs[0]) {
//        // new bar...
//        currentStep = 0;
//    }
//    lastBarInput = inputs[0];
    // monitor beat input to see wether to increase the current step:
    if (lastBeatInput > inputs[1]) {
        MidiProcessorClient::MidiEvent event;
        unsigned char channel = 1;
        unsigned char note = 50;
        unsigned char velocity = 127;

        // send note off:
        event.size = 3;
        event.buffer[0] = 0x80 + channel;
        event.buffer[1] = note;
        event.buffer[2] = velocity;
        midiProcessorClient->writeMidi(event, time);

        // send note on:
        event.size = 3;
        event.buffer[0] = 0x90 + channel;
        event.buffer[1] = note;
        event.buffer[2] = velocity;
        midiProcessorClient->writeMidi(event, time);

        currentStep++;
    }
    lastBeatInput = inputs[1];
}

void StepSequencer::setMidiProcessorClient(MidiProcessorClient *midiProcessorClient)
{
    this->midiProcessorClient = midiProcessorClient;
}
