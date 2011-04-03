#ifndef STEPSEQUENCERCLIENT_H
#define STEPSEQUENCERCLIENT_H

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

#include "parameterclient.h"
#include "stepsequencer.h"

class StepSequencerClient : public ParameterClient
{
public:
    StepSequencerClient(const QString &clientName, StepSequencer *stepSequencer, size_t ringBufferSize = 1024);

    // reimplemented from JackClient:
    virtual JackClientFactory * getFactory();

protected:
    virtual bool process(jack_nframes_t nframes);
private:
    StepSequencer *stepSequencer;
};

#endif // STEPSEQUENCERCLIENT_H
