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
    /**
      Creates a new oscillator client object with the given name.

      This object takes ownership of the given Oscillator object, i.e., it will be deleted at destruction time.
      */
    OscillatorClient(const QString &clientName, Oscillator *oscillator, size_t ringBufferSize = 1024);
    virtual ~OscillatorClient();

    virtual JackClientFactory * getFactory();
    virtual void saveState(QDataStream &stream);
    /**
      To call this method is only safe when the client is not running,
      as it accesses the internal Oscillator object used by the Jack
      process thread in a non-threadsafe way.

      To change the oscillator gain while the client is running use
      postChangeGain() method.
      */
    virtual void loadState(QDataStream &stream);

    double getGain() const;
    void postChangeGain(double gain);

    QGraphicsItem * createGraphicsItem();
private:
    Oscillator *oscillatorProcess;
    double gain;
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
