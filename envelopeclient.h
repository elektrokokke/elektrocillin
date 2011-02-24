#ifndef ENVELOPECLIENT_H
#define ENVELOPECLIENT_H

#include "envelope.h"
#include "eventprocessorclient.h"
#include "graphicsinterpolatoredititem.h"
#include <QGraphicsRectItem>
#include <QPen>
#include <QBrush>

class EnvelopeClient : public EventProcessorClient2
{
public:
    class ChangeDurationEvent : public RingBufferEvent
    {
    public:
        double duration;
    };

    EnvelopeClient(const QString &clientName, size_t ringBufferSize = (2 << 16));
    virtual ~EnvelopeClient();

    Envelope * getEnvelope();

    void postChangeDuration(double duration);

protected:
    virtual void processEvent(const RingBufferEvent *event, jack_nframes_t time);
};

class EnvelopeGraphicsItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    EnvelopeGraphicsItem(const QRectF &rect, EnvelopeClient *client, QGraphicsItem *parent = 0, const QPen &nodePen = QPen(QBrush(qRgb(114, 159, 207)), 3), const QBrush &nodeBrush = QBrush(qRgb(52, 101, 164)));
private:
    EnvelopeClient *client;
    LinearInterpolator interpolator[2];
    GraphicsInterpolatorEditItem *attackItem, *releaseItem;
};

#endif // ENVELOPECLIENT_H
