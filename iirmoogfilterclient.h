#ifndef IIRMOOGFILTERCLIENT_H
#define IIRMOOGFILTERCLIENT_H

#include "eventprocessorclient.h"
#include "eventprocessorclient.h"
#include "jackthreadeventprocessorclient.h"
#include "iirmoogfilter.h"
#include "jackringbuffer.h"
#include <QObject>

class IirMoogFilterClient;

class IirMoogFilterThread : public JackThread {
    Q_OBJECT
public:
    IirMoogFilterThread(IirMoogFilterClient *client, QObject *parent = 0);

    void setRingBufferFromClient(JackRingBuffer<IirMoogFilter::Parameters> *ringBufferFromClient);
signals:
    void changedParameters(double frequency);
protected:
    void processDeferred();
private:
    JackRingBuffer<IirMoogFilter::Parameters> *ringBufferFromClient;
};

class IirMoogFilterClient : public JackThreadEventProcessorClient<IirMoogFilter::Parameters> {
public:
    IirMoogFilterClient(const QString &clientName, IirMoogFilter *filter, size_t ringBufferSize = 1024);
    virtual ~IirMoogFilterClient();

    IirMoogFilter * getMoogFilter();
    IirMoogFilterThread * getMoogFilterThread();

protected:
    // reimplemented from EventProcessorClient:
    virtual void processEvent(const IirMoogFilter::Parameters &event, jack_nframes_t time);
    // reimplemented from MidiProcessorClient:
    virtual void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);

private:
    JackRingBuffer<IirMoogFilter::Parameters> ringBufferToThread;
};

#endif // IIRMOOGFILTERCLIENT_H
