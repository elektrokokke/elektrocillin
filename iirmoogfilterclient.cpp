#include "iirmoogfilterclient.h"

IirMoogFilterClient::IirMoogFilterClient(const QString &clientName, IirMoogFilter *filter) :
        EventProcessorClient<IirMoogFilter::Parameters>(clientName, filter)
{
}

IirMoogFilterClient::~IirMoogFilterClient()
{
    close();
}

IirMoogFilter * IirMoogFilterClient::getMoogFilter()
{
    return (IirMoogFilter*)getMidiProcessor();
}

void IirMoogFilterClient::processEvent(const IirMoogFilter::Parameters &event)
{
    getMoogFilter()->setParameters(event);
}
