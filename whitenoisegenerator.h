#ifndef WHITENOISEGENERATOR_H
#define WHITENOISEGENERATOR_H

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

#include "audioprocessorclient.h"

class WhiteNoiseGenerator : public AudioProcessor
{
public:
    WhiteNoiseGenerator(double sampleRate = 44100);

    void processAudio(const double *inputs, double *outputs, jack_nframes_t time);
};

class WhiteNoiseGeneratorClient : public AudioProcessorClient
{
public:
    WhiteNoiseGeneratorClient(const QString &clientName);
    ~WhiteNoiseGeneratorClient();

    virtual JackClientFactory * getFactory();
};

#endif // WHITENOISEGENERATOR_H
