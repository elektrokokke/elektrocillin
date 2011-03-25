#ifndef JACKTRANSPORTCLIENT_H
#define JACKTRANSPORTCLIENT_H

#include "jackthreadeventprocessorclient.h"
#include <QGraphicsSimpleTextItem>

class JackTransportClient : public JackThreadEventProcessorClient
{
public:
    JackTransportClient(const QString &clientName, size_t ringBufferSize = 1024);
    virtual ~JackTransportClient();

    virtual JackClientFactory * getFactory();

    QGraphicsItem * createGraphicsItem();
protected:
    // reimplemented from EventProcessorClient:
    virtual bool process(jack_nframes_t nframes);

private:
    JackRingBuffer<jack_position_t> ringBufferToThread;
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

class JackTransportGraphicsItem : public QObject, public QGraphicsSimpleTextItem
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
