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
    EnvelopeClient(const QString &clientName, size_t ringBufferSize = (2 << 16));
    virtual ~EnvelopeClient();

    virtual JackClientFactory * getFactory();

    Envelope * getEnvelope();

    void postIncreaseControlPoints();
    void postDecreaseControlPoints();
    void postChangeControlPoint(int index, double x, double y);
    void postChangeDuration(double duration);
    void postChangeSustainPosition(double sustainPosition);

    QGraphicsItem * createGraphicsItem(const QRectF &rect);

protected:
    virtual void processEvent(const RingBufferEvent *event, jack_nframes_t time);
private:
    Envelope *envelope, *envelopeProcess;
};

class EnvelopeGraphicsItem;

class EnvelopeGraphicsSubItem : public GraphicsInterpolatorEditItem
{
    Q_OBJECT
public:
    EnvelopeGraphicsSubItem(const QRectF &rect, EnvelopeGraphicsItem *parent, const QPen &nodePen = QPen(QBrush(qRgb(114, 159, 207)), 3), const QBrush &nodeBrush = QBrush(qRgb(52, 101, 164)));
protected:
    virtual void increaseControlPoints();
    virtual void decreaseControlPoints();
    virtual void changeControlPoint(int index, int nrOfControlPoints, double x, double y);
private:
    EnvelopeGraphicsItem *parent;
};

class EnvelopeGraphicsItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    EnvelopeGraphicsItem(const QRectF &rect, EnvelopeClient *client, QGraphicsItem *parent = 0, const QPen &nodePen = QPen(QBrush(qRgb(114, 159, 207)), 3), const QBrush &nodeBrush = QBrush(qRgb(52, 101, 164)));

    EnvelopeClient * getClient();

    void changeSustainLevel(double sustainLevel);
private slots:
    void onDurationNodePositionChanged(qreal x);
    void onSustainNodePositionChanged(qreal x);
private:
    EnvelopeClient *client;
    EnvelopeGraphicsSubItem *interpolatorEditItem;
    GraphicsNodeItem *nodeItemDuration, *nodeItemSustainPosition;
    QGraphicsLineItem *sustainPositionLine;
    QGraphicsSimpleTextItem *sustainPositionText, *durationText;
};

#endif // ENVELOPECLIENT_H
