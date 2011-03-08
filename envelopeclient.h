#ifndef ENVELOPECLIENT_H
#define ENVELOPECLIENT_H

#include "envelope.h"
#include "eventprocessorclient.h"
#include "graphicsinterpolatoredititem.h"
#include <QGraphicsRectItem>
#include <QPen>
#include <QBrush>
#include <QFont>

class EnvelopeClient : public EventProcessorClient
{
public:
    EnvelopeClient(const QString &clientName, size_t ringBufferSize = (2 << 16));
    virtual ~EnvelopeClient();

    virtual JackClientFactory * getFactory();
    virtual void saveState(QDataStream &stream);
    virtual void loadState(QDataStream &stream);

    Envelope * getEnvelope();

    void postIncreaseControlPoints();
    void postDecreaseControlPoints();
    void postChangeControlPoint(int index, double x, double y);
    void postChangeSustainIndex(int sustainIndex);

    QGraphicsItem * createGraphicsItem();

protected:
    virtual void processEvent(const RingBufferEvent *event, jack_nframes_t time);
private:
    Envelope *envelope, *envelopeProcess;
};

class EnvelopeGraphicsItem : public QObject, public GraphicsInterpolatorEditItem
{
    Q_OBJECT
public:
    EnvelopeGraphicsItem(const QRectF &rect, EnvelopeClient *client, QGraphicsItem *parent = 0);
    EnvelopeClient * getClient();
protected:
    virtual void increaseControlPoints();
    virtual void decreaseControlPoints();
    virtual void changeControlPoint(int index, double x, double y);
private:
    EnvelopeClient *client;
};

#endif // ENVELOPECLIENT_H
