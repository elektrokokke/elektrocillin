#ifndef CHAMBERLINFILTER_H
#define CHAMBERLINFILTER_H

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
#include "parameterclient.h"

class ChamberlinFilter : public AudioProcessor, public MidiParameterProcessor
{
public:
    ChamberlinFilter();

    // Reimplemented from AudioProcessor:
    virtual void setSampleRate(double sampleRate);
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);

private:
    double f, low, high, band, notch, scale;
};

class ChamberlinFilterClient : public ParameterClient
{
public:
    ChamberlinFilterClient(const QString &clientName, ChamberlinFilter *processFilter, ChamberlinFilter *guiFilter, size_t ringBufferSize = 1024);
    virtual ~ChamberlinFilterClient();
    virtual JackClientFactory * getFactory();
protected:
    // reimplemented from ParameterClient:
    virtual bool init();
private:
    ChamberlinFilter *processFilter, *guiFilter;
};

#endif // CHAMBERLINFILTER_H
