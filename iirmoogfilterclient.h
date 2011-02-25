#ifndef IIRMOOGFILTERCLIENT_H
#define IIRMOOGFILTERCLIENT_H

#include "eventprocessorclient.h"
#include "eventprocessorclient.h"
#include "jackthreadeventprocessorclient.h"
#include "iirmoogfilter.h"
#include "jackringbuffer.h"
#include "frequencyresponsegraphicsitem.h"
#include "graphicsnodeitem.h"
#include <QObject>

class IirMoogFilterClient;

class IirMoogFilterThread : public JackThread {
    Q_OBJECT
public:
    IirMoogFilterThread(IirMoogFilterClient *client, QObject *parent = 0);

    void setRingBufferFromClient(JackRingBuffer<IirMoogFilter::Parameters> *ringBufferFromClient);
signals:
    void changedParameters(double frequency, double resonance);
protected:
    void processDeferred();
private:
    JackRingBuffer<IirMoogFilter::Parameters> *ringBufferFromClient;
};

class IirMoogFilterClient : public JackThreadEventProcessorClient<IirMoogFilter::Parameters>
{
public:
    IirMoogFilterClient(const QString &clientName, size_t ringBufferSize = 1024);
    virtual ~IirMoogFilterClient();

    virtual JackClientFactory * getFactory();

    IirMoogFilter * getMoogFilter();
    IirMoogFilterThread * getMoogFilterThread();

    QGraphicsItem * createGraphicsItem(const QRectF &rect);
protected:
    // reimplemented from EventProcessorClient:
    virtual void processEvent(const IirMoogFilter::Parameters &event, jack_nframes_t time);
    // reimplemented from MidiProcessorClient:
    virtual void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    virtual void processController(unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time);

private:
    JackRingBuffer<IirMoogFilter::Parameters> ringBufferToThread;
};

class IirMoogFilterGraphicsItem : public QObject, public FrequencyResponseGraphicsItem
{
    Q_OBJECT
public:
    IirMoogFilterGraphicsItem(IirMoogFilterClient *client, const QRectF &rect, QGraphicsItem *parent = 0);
private:
    IirMoogFilterClient *client;
    IirMoogFilter filterCopy;
    GraphicsNodeItem *cutoffResonanceNode;
private slots:
    void onGuiChangedFilterParameters(const QPointF &cutoffResonance);
    void onClientChangedFilterParameters(double frequency, double resonance);
};

#endif // IIRMOOGFILTERCLIENT_H
