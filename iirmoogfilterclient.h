#ifndef IIRMOOGFILTERCLIENT_H
#define IIRMOOGFILTERCLIENT_H

#include "eventprocessorclient.h"
#include "eventprocessorclient.h"
#include "iirmoogfilter.h"
#include "jackringbuffer.h"
#include <QObject>

class IIRMoogFilterClient : public EventProcessorClient<IIRMoogFilter::Parameters> {
public:
    IIRMoogFilterClient(const QString &clientName, IIRMoogFilter *filter);
    virtual ~IIRMoogFilterClient();

    IIRMoogFilter * getMoogFilter();

protected:
    // reimplemented method from EventProcessorClient:
    virtual void processEvent(const IIRMoogFilter::Parameters &event);
};

struct IIRMoogFilterControl {
    jack_nframes_t time;
    IIRMoogFilter::Parameters parameters;
};

class IIRMoogFilterClientOld : public AudioProcessorClient
{
public:
    IIRMoogFilterClientOld(const QString &clientName, IIRMoogFilter *filter);
    virtual ~IIRMoogFilterClientOld();

    // this method can be called from any thread:
    void setParameters(const IIRMoogFilter::Parameters &parameters);

protected:
    // reimplemented methods from AudioProcessorClient:
    virtual bool init();
    virtual bool process(jack_nframes_t nframes);

private:
    IIRMoogFilter *filter;
    JackRingBuffer<IIRMoogFilterControl> controlRingBuffer;
};

#endif // IIRMOOGFILTERCLIENT_H
