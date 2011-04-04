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

#include "multiplyprocessor.h"

MultiplyProcessor::MultiplyProcessor(double gain_, double sampleRate) :
    AudioProcessor(QStringList("Factor 1") + QStringList("Factor 2"), QStringList("Product out"), sampleRate),
    gain(gain_)
{
}

MultiplyProcessor::MultiplyProcessor(const QStringList &inputPortNames, double gain_, double sampleRate) :
    AudioProcessor(inputPortNames, QStringList("Product"), sampleRate),
    gain(gain_)
{
}

void MultiplyProcessor::setGainFactor(double gain)
{
    this->gain = gain;
}

double MultiplyProcessor::getGainFactor() const
{
    return gain;
}

void MultiplyProcessor::processAudio(const double *inputs, double *outputs, jack_nframes_t)
{
    // multiply all inputs:
    double product = gain;
    for (int i = 0; i < getNrOfInputs(); i++) {
        product *= inputs[i];
    }
    outputs[0] = product;
}

MultiplyClient::MultiplyClient(const QString &clientName) :
    AudioProcessorClient(clientName, new MultiplyProcessor())
{
}

MultiplyClient::~MultiplyClient()
{
    close();
    delete getAudioProcessor();
}

MultiplyProcessor * MultiplyClient::getMultiplyProcessor()
{
    return (MultiplyProcessor*)getAudioProcessor();
}

void MultiplyClient::saveState(QDataStream &stream)
{
    AudioProcessorClient::saveState(stream);
    stream << getMultiplyProcessor()->getGainFactor();
}

void MultiplyClient::loadState(QDataStream &stream)
{
    AudioProcessorClient::loadState(stream);
    double gain;
    stream >> gain;
    getMultiplyProcessor()->setGainFactor(gain);
}

class MultiplyClientFactory : public JackClientFactory
{
public:
    MultiplyClientFactory()
    {
        JackClientSerializer::getInstance()->registerFactory(this);
    }
    QString getName()
    {
        return "Multiplier";
    }
    JackClient * createClient(const QString &clientName)
    {
        return new MultiplyClient(clientName);
    }
    static MultiplyClientFactory factory;
};

MultiplyClientFactory MultiplyClientFactory::factory;

JackClientFactory * MultiplyClient::getFactory()
{
    return &MultiplyClientFactory::factory;
}
