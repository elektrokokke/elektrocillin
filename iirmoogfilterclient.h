#ifndef IIRMOOGFILTERCLIENT_H
#define IIRMOOGFILTERCLIENT_H

#include "audioprocessorclient.h"
#include "iirmoogfilter.h"
#include "jackringbuffer.h"
#include <QObject>

struct IIRMoogFilterControl {
    jack_nframes_t time;
    double cutoffFrequency;
    double resonance;
};

class IIRMoogFilterClient : public QObject, public AudioProcessorClient
{
    Q_OBJECT
public:
    IIRMoogFilterClient(const QString &clientName, IIRMoogFilter *filter, QObject *parent = 0);
    virtual ~IIRMoogFilterClient();

public slots:
    void setParameters(double cutoffFrequency, double resonance);

protected:
    // reimplemented methods from AudioProcessorClient:
    virtual bool init();
    virtual bool process(jack_nframes_t nframes);

private:
    IIRMoogFilter *filter;
    JackRingBuffer<IIRMoogFilterControl> controlRingBuffer;
};

#endif // IIRMOOGFILTERCLIENT_H
