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

#include "stepsequencerclient.h"

StepSequencerClient::StepSequencerClient(const QString &clientName, StepSequencer *stepSequencer, size_t ringBufferSize) :
    ParameterClient(clientName, stepSequencer, 0, 0, stepSequencer, ringBufferSize)
{
    stepSequencer->setMidiProcessorClient(this);
    activateMidiInput(false);
    activateMidiOutput(true);
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
        return new StepSequencerClient(clientName, new StepSequencer(16));
    }
    static StepSequencerClientFactory factory;
};

StepSequencerClientFactory StepSequencerClientFactory::factory;

JackClientFactory * StepSequencerClient::getFactory()
{
    return &StepSequencerClientFactory::factory;
}
