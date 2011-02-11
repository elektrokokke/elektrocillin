#include "adsrclient.h"

AdsrClient::AdsrClient(const QString &clientName, const AdsrParameters &parameters, size_t ringBufferSize) :
    EventProcessorClient<AdsrParameters>(clientName, new AdsrEnvelope(parameters.release, parameters.decay, parameters.sustain, parameters.release), ringBufferSize)
{
}

AdsrClient::AdsrClient(const QString &clientName, double attackTime, double decayTime, double sustainLevel, double releaseTime, size_t ringBufferSize) :
    EventProcessorClient<AdsrParameters>(clientName, new AdsrEnvelope(attackTime, decayTime, sustainLevel, releaseTime), ringBufferSize)
{
}

AdsrClient::~AdsrClient()
{
    close();
}

AdsrEnvelope * AdsrClient::getAdsrEnvelope()
{
    return (AdsrEnvelope*)getMidiProcessor();
}

void AdsrClient::processEvent(const AdsrParameters &event, jack_nframes_t)
{
    getAdsrEnvelope()->setAttackTime(event.attack);
    getAdsrEnvelope()->setDecayTime(event.decay);
    getAdsrEnvelope()->setSustainLevel(event.sustain);
    getAdsrEnvelope()->setReleaseTime(event.release);
}
