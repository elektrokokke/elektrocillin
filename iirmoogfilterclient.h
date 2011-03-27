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

class IirMoogFilterClient : public JackThreadEventProcessorClient
{
    Q_OBJECT
public:
    /**
      Creates a new Moog filter client object with the given name. An associated IirMoogFilterThread will be
      automatically created and also deleted at destruction time.

      This object takes ownership of the given IirMoogFilter object, i.e., it will be deleted at destruction time.
      */
    IirMoogFilterClient(const QString &clientName, IirMoogFilter *filter, size_t ringBufferSize = 1024);
    virtual ~IirMoogFilterClient();

    virtual JackClientFactory * getFactory();
    virtual void saveState(QDataStream &stream);
    /**
      To call this method is only safe when the client is not running,
      as it accesses the internal IirMoogFilter object used by the Jack
      process thread in a non-threadsafe way.

      To change the filter state while the client is running use
      postEvent() with a IirMoogFilter::Parameters object.
      */
    virtual void loadState(QDataStream &stream);

    IirMoogFilter * getMoogFilter();
    IirMoogFilterThread * getMoogFilterThread();

    QGraphicsItem * createGraphicsItem();
protected:
    /**
      This method has been reimplemented from MidiProcessorClient to notify
      the associated IirMoogFilterThread of changes to the filter parameters
      that occur through MIDI events.
      */
    virtual void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    /**
      This method has been reimplemented from MidiProcessorClient to notify
      the associated IirMoogFilterThread of changes to the filter parameters
      that occur through MIDI events.
      */
    virtual void processController(unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time);
private slots:
    void onClientChangedFilterParameters(double frequency, double resonance);
private:
    IirMoogFilter *iirMoogFilterProcess, *iirMoogFilter;
    JackRingBuffer<IirMoogFilter::Parameters> ringBufferToThread;
};

class IirMoogFilterGraphicsItem : public QObject, public FrequencyResponseGraphicsItem
{
    Q_OBJECT
public:
    IirMoogFilterGraphicsItem(IirMoogFilterClient *client, const QRectF &rect, QGraphicsItem *parent = 0);
private:
    IirMoogFilterClient *client;
    GraphicsNodeItem *cutoffResonanceNode;
private slots:
    void onGuiChangedFilterParameters(const QPointF &cutoffResonance);
    void onClientChangedFilterParameters(double frequency, double resonance);
};

#endif // IIRMOOGFILTERCLIENT_H
