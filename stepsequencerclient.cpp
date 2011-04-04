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

#include "stepsequencerclient.h"

StepSequencerClient::StepSequencerClient(const QString &clientName, StepSequencer *processStepSequencer_, StepSequencer *guiStepSequencer_, size_t ringBufferSize) :
    ParameterClient(clientName, processStepSequencer_, 0, 0, processStepSequencer_, guiStepSequencer_, ringBufferSize),
    processStepSequencer(processStepSequencer_),
    guiStepSequencer(guiStepSequencer_)
{
    processStepSequencer->setMidiProcessorClient(this);
    activateMidiInput(false);
    activateMidiOutput(true);
}

bool StepSequencerClient::process(jack_nframes_t nframes)
{
    // get the current transport state and send it to the associated thread:
    jack_position_t position;
    jack_transport_state_t state = jack_transport_query(getClient(), &position);
    processStepSequencer->setTransportPosition(state, position);
    return ParameterClient::process(nframes);
}

class StepSequencerClientFactory : public JackClientFactory
{
public:
    StepSequencerClientFactory()
    {
        JackClientSerializer::getInstance()->registerFactory(this);
    }
    QString getName()
    {
        return "Step sequencer";
    }
    JackClient * createClient(const QString &clientName)
    {
        return new StepSequencerClient(clientName, new StepSequencer(16), new StepSequencer(16));
    }
    static StepSequencerClientFactory factory;
};

StepSequencerClientFactory StepSequencerClientFactory::factory;

JackClientFactory * StepSequencerClient::getFactory()
{
    return &StepSequencerClientFactory::factory;
}
