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

#endif // IIRMOOGFILTERCLIENT_H
