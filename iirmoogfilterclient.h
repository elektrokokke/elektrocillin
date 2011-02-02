#ifndef IIRMOOGFILTERCLIENT_H
#define IIRMOOGFILTERCLIENT_H

#include "jackclientwithdeferredprocessing.h"
#include "jackthread.h"
#include "iirmoogfilter.h"

struct IIRMoogFilterControl {
    jack_nframes_t time;
    double cutoffFrequency;
    double resonance;
};

class IIRMoogFilterClient : public QObject, public JackClient
{
    Q_OBJECT
public:
    IIRMoogFilterClient(const QString &clientName, QObject *parent = 0);
    virtual ~IIRMoogFilterClient();

public slots:
    void setParameters(double cutoffFrequency, double resonance);

protected:
    // reimplemented methods from JackClient:
    virtual bool init();
    virtual bool process(jack_nframes_t nframes);

private:
    JackRingBuffer<IIRMoogFilterControl> controlRingBuffer;
    IIRMoogFilter filter;
    QString audioInputPortName, audioOutputPortName;
    jack_port_t *audioInputPort, *audioOutputPort;
};

#endif // IIRMOOGFILTERCLIENT_H
