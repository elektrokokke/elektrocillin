#ifndef JACKTRANSPORTCLIENT_H
#define JACKTRANSPORTCLIENT_H

#include "jackthreadeventprocessorclient.h"
#include "graphicslabelitem.h"

class JackTransportClient : public JackThreadEventProcessorClient
{
public:
    JackTransportClient(const QString &clientName, size_t ringBufferSize = 1024);
    virtual ~JackTransportClient();

    virtual JackClientFactory * getFactory();

    QGraphicsItem * createGraphicsItem();
protected:
    // reimplemented from JackThreadEventProcessorClient:
    virtual bool init();
    // reimplemented from EventProcessorClient:
    virtual bool process(jack_nframes_t nframes);
    virtual void timebase(jack_transport_state_t state, jack_nframes_t nframes, jack_position_t *pos, int new_pos);
private:
    JackRingBuffer<jack_position_t> ringBufferToThread;
    double beatsPerMinute;
    int beatsPerBar, beatType, ticksPerBeat;

    static void timebase(jack_transport_state_t state, jack_nframes_t nframes, jack_position_t *pos, int new_pos, void *arg);
};

class JackTransportThread : public JackThread {
    Q_OBJECT
public:
    JackTransportThread(JackTransportClient *client, QObject *parent = 0);

    void setRingBufferFromClient(JackRingBuffer<jack_position_t> *ringBufferFromClient);
signals:
    void changedPosition(const QString &pos);
protected:
    void processDeferred();
private:
    JackRingBuffer<jack_position_t> *ringBufferFromClient;
};

class JackTransportGraphicsItem : public QObject, public GraphicsLabelItem
{
    Q_OBJECT
public:
    JackTransportGraphicsItem(QGraphicsItem *parent = 0);
public slots:
    void changePosition(const QString &pos);
private:
    JackTransportClient *client;
};


#endif // JACKTRANSPORTCLIENT_H
