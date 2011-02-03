#ifndef IIRMOOGFILTERCLIENT_H
#define IIRMOOGFILTERCLIENT_H

#include "eventprocessorclient.h"
#include "audioprocessorclient.h"
#include "iirmoogfilter.h"
#include "jackringbuffer.h"
#include <QObject>

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
