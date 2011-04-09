#ifndef CHAMBERLINFILTER_H
#define CHAMBERLINFILTER_H

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
