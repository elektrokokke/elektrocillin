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

#include "whitenoisegenerator.h"
#include <cstdlib>

WhiteNoiseGenerator::WhiteNoiseGenerator(double sampleRate) :
    AudioProcessor(QStringList(), QStringList("Noise out"), sampleRate)
{
}

void WhiteNoiseGenerator::processAudio(const double *, double *outputs, jack_nframes_t)
{
    int randomNumber = rand();
    outputs[0] = (double)randomNumber / (double)RAND_MAX * 2.0 - 1.0;
}

WhiteNoiseGeneratorClient::WhiteNoiseGeneratorClient(const QString &clientName) :
    AudioProcessorClient(clientName, new WhiteNoiseGenerator(44100))
{
}

WhiteNoiseGeneratorClient::~WhiteNoiseGeneratorClient()
{
    close();
    delete getAudioProcessor();
}

class WhiteNoiseGeneratorClientFactory : public JackClientFactory
{
public:
    WhiteNoiseGeneratorClientFactory()
    {
        JackClientSerializer::getInstance()->registerFactory(this);
    }
    QString getName()
    {
        return "White noise generator";
    }
    JackClient * createClient(const QString &clientName)
    {
        return new WhiteNoiseGeneratorClient(clientName);
    }
    static WhiteNoiseGeneratorClientFactory factory;
};

WhiteNoiseGeneratorClientFactory WhiteNoiseGeneratorClientFactory::factory;

JackClientFactory * WhiteNoiseGeneratorClient::getFactory()
{
    return &WhiteNoiseGeneratorClientFactory::factory;
}
