#ifndef IIRBUTTERWORTHFILTER_H
#define IIRBUTTERWORTHFILTER_H

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

#include "iirfilter.h"
#include "midiparameterprocessor.h"
#include "parameterclient.h"

class IirButterworthFilter2 : public IirFilter
{
public:
    enum Type {
        LOW_PASS,
        HIGH_PASS
    };

    IirButterworthFilter2(double cutoffFrequencyInHertz, Type type = LOW_PASS);

    virtual void setCutoffFrequency(double cutoffFrequencyInHertz, Type type);

    void setCutoffFrequency(double cutoffFrequencyInHertz);
    void setType(Type type);

    double getCutoffFrequency() const;
    Type getType() const;

private:
    double cutoffFrequency;
    Type type;
};

class IirButterworthFilter : public AudioProcessor, public MidiParameterProcessor
{
public:
    IirButterworthFilter();

    // Reimplemented from AudioProcessor:
    virtual void setSampleRate(double sampleRate);
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);
    // Reimplemented from MidiProcessor:
    virtual void processNoteOn(int inputIndex, unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    virtual void processPitchBend(int inputIndex, unsigned char channel, unsigned int value, jack_nframes_t time);
    // Reimplemented from MidiParameterProcessor:
    virtual bool setParameterValue(int index, double value, double min, double max, unsigned int time);

    FrequencyResponse * getLowpassResponse();
    FrequencyResponse * getHighpassResponse();
    FrequencyResponse * getBandpassResponse();
private:
    IirButterworthFilter2 lowpass, highpass;
    IirFilter bandpass;

    void computeCoefficients();
};

class IirButterworthFilterClient : public ParameterClient
{
    Q_OBJECT
public:
    IirButterworthFilterClient(const QString &clientName, IirButterworthFilter *processFilter, IirButterworthFilter *guiFilter, size_t ringBufferSize = 1024);
    virtual ~IirButterworthFilterClient();
    virtual QGraphicsItem * createGraphicsItem();
    virtual JackClientFactory * getFactory();
protected:
    // reimplemented from ParameterClient:
    virtual bool init();
private:
    IirButterworthFilter *processFilter, *guiFilter;
};

#endif // IIRBUTTERWORTHFILTER_H
