#ifndef OSCILLATORCLIENT_H
#define OSCILLATORCLIENT_H

#include "eventprocessorclient.h"
#include "oscillator.h"
#include <QGraphicsRectItem>
#include <QBrush>
#include <QPen>

class OscillatorClient : public EventProcessorClient
{
public:
    class ChangeGainEvent : public RingBufferEvent
    {
    public:
        double gain;
    };

    OscillatorClient(const QString &clientName, Oscillator *oscillator, size_t ringBufferSize = 1024);
    OscillatorClient(const QString &clientName, size_t ringBufferSize = 1024);
    virtual ~OscillatorClient();

    virtual JackClientFactory * getFactory();
    virtual void saveState(QDataStream &stream);
    virtual void loadState(QDataStream &stream);

    Oscillator * getOscillator();

    void postChangeGain(double gain);

    QGraphicsItem * createGraphicsItem();

protected:
    virtual void processEvent(const RingBufferEvent *event, jack_nframes_t time);

private:
    Oscillator *oscillator;
};

class OscillatorClientGraphicsItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    OscillatorClientGraphicsItem(const QRectF &rect, OscillatorClient *client, QGraphicsItem *parent = 0, const QPen &nodePen = QPen(QBrush(qRgb(114, 159, 207)), 3), const QBrush &nodeBrush = QBrush(qRgb(52, 101, 164)));
private slots:
    void onNodeYChanged(qreal y);
private:
    OscillatorClient *client;
};

#endif // OSCILLATORCLIENT_H
