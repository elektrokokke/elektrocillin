#ifndef IIRMOOGFILTERCLIENT_H
#define IIRMOOGFILTERCLIENT_H

#include "jackclientwithdeferredprocessing.h"
#include "jackthread.h"
#include "iirmoogfilter.h"

struct IIRMoogFilterControl {
    double cutoffFrequency;
    double resonance;
};

class IIRMoogFilterControlThread : public JackThread
{
public:
    IIRMoogFilterControlThread(JackClientWithDeferredProcessing *client, QObject *parent = 0);

    JackRingBuffer<IIRMoogFilterControl> * getOutputRingBuffer();

protected:
    virtual void processDeferred();

private:
    JackRingBuffer<IIRMoogFilterControl> ringBufferToClient;
};

class IIRMoogFilterClient : public JackClientWithDeferredProcessing
{
public:
    IIRMoogFilterClient(const QString &clientName);

protected:
    IIRMoogFilterControlThread * getControlThread();
    JackRingBuffer<IIRMoogFilterControl> * getInputRingBuffer();
    // reimplemented methods from JackClient:
    virtual bool init();
    virtual bool process(jack_nframes_t nframes);

private:
    IIRMoogFilterControlThread thread;
    IIRMoogFilter filter;
};

#endif // IIRMOOGFILTERCLIENT_H
