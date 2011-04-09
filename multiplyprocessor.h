#ifndef MULTIPLYPROCESSOR_H
#define MULTIPLYPROCESSOR_H

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

#include "audioprocessorclient.h"

class MultiplyProcessor : public AudioProcessor
{
public:
    MultiplyProcessor(double gain = 1.0);
    MultiplyProcessor(const QStringList &audioInputPortNames, double gain = 1.0);

    void setGainFactor(double gain);
    double getGainFactor() const;

    void processAudio(const double *inputs, double *outputs, jack_nframes_t time);

private:
    double gain;
};

class MultiplyClient : public AudioProcessorClient
{
public:
    MultiplyClient(const QString &clientName);
    ~MultiplyClient();

    MultiplyProcessor * getMultiplyProcessor();

    virtual JackClientFactory * getFactory();
    virtual void saveState(QDataStream &stream);
    virtual void loadState(QDataStream &stream);
};

#endif // MULTIPLYPROCESSOR_H
