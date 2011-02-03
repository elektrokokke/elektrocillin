#ifndef IIRMOOGFILTERCLIENT_H
#define IIRMOOGFILTERCLIENT_H

#include "eventprocessorclient.h"
#include "eventprocessorclient.h"
#include "iirmoogfilter.h"
#include "jackringbuffer.h"
#include <QObject>

class IirMoogFilterClient : public EventProcessorClient<IirMoogFilter::Parameters> {
public:
    IirMoogFilterClient(const QString &clientName, IirMoogFilter *filter);
    virtual ~IirMoogFilterClient();

    IirMoogFilter * getMoogFilter();

protected:
    // reimplemented method from EventProcessorClient:
    virtual void processEvent(const IirMoogFilter::Parameters &event);
};

struct IIRMoogFilterControl {
    jack_nframes_t time;
    IirMoogFilter::Parameters parameters;
};

class IIRMoogFilterClientOld : public AudioProcessorClient
{
public:
    IIRMoogFilterClientOld(const QString &clientName, IirMoogFilter *filter);
    virtual ~IIRMoogFilterClientOld();

    // this method can be called from any thread:
    void setParameters(const IirMoogFilter::Parameters &parameters);

protected:
    // reimplemented methods from AudioProcessorClient:
    virtual bool init();
    virtual bool process(jack_nframes_t nframes);

private:
    IirMoogFilter *filter;
    JackRingBuffer<IIRMoogFilterControl> controlRingBuffer;
};

#endif // IIRMOOGFILTERCLIENT_H
