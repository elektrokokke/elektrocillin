#ifndef ADSRCLIENT_H
#define ADSRCLIENT_H

#include "eventprocessorclient.h"
#include "adsrenvelope.h"

struct AdsrParameters {
    double attack, decay, sustain, release;
};

class AdsrClient : public EventProcessorClient<AdsrParameters>
{
public:
    AdsrClient(const QString &clientName, const AdsrParameters &parameters, size_t ringBufferSize = 1024);
    AdsrClient(const QString &clientName, double attackTime, double decayTime, double sustainLevel, double releaseTime, size_t ringBufferSize = 1024);
    virtual ~AdsrClient();

    AdsrEnvelope * getAdsrEnvelope();

protected:
    virtual void processEvent(const AdsrParameters &event, jack_nframes_t time);
};

#endif // ADSRCLIENT_H
