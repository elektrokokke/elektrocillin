#include "envelopeclient.h"
#include <math.h>

EnvelopeClient::EnvelopeClient(const QString &clientName, size_t ringBufferSize) :
    EventProcessorClient(clientName, new Envelope(), ringBufferSize)
{
    envelopeProcess = (Envelope*)getAudioProcessor();
    envelope = new Envelope();
}

EnvelopeClient::~EnvelopeClient()
{
    close();
    delete envelope;
    delete envelopeProcess;
}

void EnvelopeClient::saveState(QDataStream &stream)
{
    envelope->getInterpolator()->save(stream);
    stream << envelope->getSustainPosition();
}

void EnvelopeClient::loadState(QDataStream &stream)
{
    LinearInterpolator interpolator;
    interpolator.load(stream);
    envelope->setInterpolator(interpolator);
    envelopeProcess->setInterpolator(interpolator);
    double sustainPosition;
    stream >> sustainPosition;
    envelope->setSustainPosition(sustainPosition);
    envelopeProcess->setSustainPosition(sustainPosition);
}

Envelope * EnvelopeClient::getEnvelope()
{
    return envelope;
}

void EnvelopeClient::postIncreaseControlPoints()
{
    LinearInterpolator *interpolator = envelope->getInterpolator();
    int size = interpolator->getX().size() + 1;
    double stretchFactor = (double)(interpolator->getX().size() - 1) / (double)(size - 1);
    interpolator->getX().append(interpolator->getX().back());
    interpolator->getY().append(interpolator->getY().back());
    for (int i = size - 1; i >= 0; i--) {
        if (i < size - 1) {
            interpolator->getX()[i] = interpolator->getX()[i] * stretchFactor;
        }
    }
    Interpolator::ChangeAllControlPointsEvent *event = new Interpolator::ChangeAllControlPointsEvent();
    event->xx = interpolator->getX();
    event->yy = interpolator->getY();
    postEvent(event);
}

void EnvelopeClient::postDecreaseControlPoints()
{
    LinearInterpolator *interpolator = envelope->getInterpolator();
    if (interpolator->getX().size() > 2) {
        int size = interpolator->getX().size() - 1;
        double stretchFactor = interpolator->getX().back() / interpolator->getX()[size - 1];
        interpolator->getX().resize(size);
        interpolator->getY().resize(size);
        for (int i = size - 1; i >= 0; i--) {
            interpolator->getX()[i] = interpolator->getX()[i] * stretchFactor;
        }
        interpolator->getY().back() = 0;
        Interpolator::ChangeAllControlPointsEvent *event = new Interpolator::ChangeAllControlPointsEvent();
        event->xx = interpolator->getX();
        event->yy = interpolator->getY();
        postEvent(event);
    }
}

void EnvelopeClient::postChangeControlPoint(int index, double x, double y)
{
    LinearInterpolator *interpolator = envelope->getInterpolator();
    if (index == 0) {
       x = interpolator->getX()[0];
    }
    if (index == interpolator->getX().size() - 1) {
        x = interpolator->getX().back();
    }
    if ((index > 0) && (x <= interpolator->getX()[index - 1])) {
        x = interpolator->getX()[index - 1];
    }
    if ((index < interpolator->getX().size() - 1) && (x >= interpolator->getX()[index + 1])) {
        x = interpolator->getX()[index + 1];
    }
    Interpolator::ChangeControlPointEvent *event = new Interpolator::ChangeControlPointEvent();
    event->index = index;
    interpolator->getX()[index] = event->x = x;
    interpolator->getY()[index] = event->y = y;
    postEvent(event);
}

void EnvelopeClient::postChangeDuration(double duration)
{
    Envelope::ChangeDurationEvent *event = new Envelope::ChangeDurationEvent();
    event->duration = duration;
    envelope->processEvent(event, 0);
    postEvent(event);
}

void EnvelopeClient::postChangeSustainPosition(double sustainPosition)
{
    Envelope::ChangeSustainPositionEvent *event = new Envelope::ChangeSustainPositionEvent();
    event->sustainPosition = sustainPosition;
    envelope->processEvent(event, 0);
    postEvent(event);
}

QGraphicsItem * EnvelopeClient::createGraphicsItem(const QRectF &rect)
{
    return new EnvelopeGraphicsItem(rect, this);
}

void EnvelopeClient::processEvent(const RingBufferEvent *event, jack_nframes_t time)
{
    if (const Interpolator::ChangeControlPointEvent *changeControlPointEvent = dynamic_cast<const Interpolator::ChangeControlPointEvent*>(event)) {
        envelopeProcess->processEvent(changeControlPointEvent, time);
    } else if (const Interpolator::ChangeAllControlPointsEvent *changeAllControlPointsEvent = dynamic_cast<const Interpolator::ChangeAllControlPointsEvent*>(event)) {
        envelopeProcess->processEvent(changeAllControlPointsEvent, time);
    } else if (const Envelope::ChangeDurationEvent *changeDurationEvent = dynamic_cast<const Envelope::ChangeDurationEvent*>(event)) {
        envelopeProcess->processEvent(changeDurationEvent, time);
    } else if (const Envelope::ChangeSustainPositionEvent *changeSustainPositionEvent = dynamic_cast<const Envelope::ChangeSustainPositionEvent*>(event)) {
        envelopeProcess->processEvent(changeSustainPositionEvent, time);
    }
}

EnvelopeGraphicsSubItem::EnvelopeGraphicsSubItem(const QRectF &rect, EnvelopeGraphicsItem *parent_, const QPen &nodePen, const QBrush &nodeBrush) :
    GraphicsInterpolatorEditItem(parent_->getClient()->getEnvelope()->getInterpolator(), rect, QRectF(0, 1, parent_->getClient()->getEnvelope()->getDuration(), -2), parent_, 8, 8, nodePen, nodeBrush),
    parent(parent_)
{
    setVisible(GraphicsInterpolatorEditItem::FIRST, false);
    setVisible(GraphicsInterpolatorEditItem::LAST, false);
}

void EnvelopeGraphicsSubItem::increaseControlPoints()
{
    parent->getClient()->postIncreaseControlPoints();
}

void EnvelopeGraphicsSubItem::decreaseControlPoints()
{
    parent->getClient()->postDecreaseControlPoints();
}

void EnvelopeGraphicsSubItem::changeControlPoint(int index, double x, double y)
{
    parent->getClient()->postChangeControlPoint(index, x, y);
}

EnvelopeGraphicsItem::EnvelopeGraphicsItem(const QRectF &rect, EnvelopeClient *client_, QGraphicsItem *parent, const QPen &nodePen, const QBrush &nodeBrush) :
        QGraphicsRectItem(rect, parent),
        client(client_)
{
    setPen(QPen(Qt::NoPen));
    setBrush(QBrush(Qt::white));

    QGraphicsRectItem *rectDuration = new QGraphicsRectItem(rect.left(), rect.bottom() - 16, rect.width(), 16, this);
    rectDuration->setPen(QPen(QBrush(Qt::black), 2));
    QGraphicsRectItem *rectSustain = new QGraphicsRectItem(rect.left(), rect.bottom() - 32, rect.width(), 16, this);
    rectSustain->setPen(QPen(QBrush(Qt::black), 2));

    interpolatorEditItem = new EnvelopeGraphicsSubItem(QRectF(rect.x(), rect.y(), rect.width(), rect.height() - 32), this, nodePen, nodeBrush);
    interpolatorEditItem->setBrush(QBrush(Qt::NoBrush));

    nodeItemDuration = new GraphicsNodeItem(-5.0, -5.0, 10.0, 10.0, this);
    nodeItemDuration->setPen(nodePen);
    nodeItemDuration->setBrush(nodeBrush);
    nodeItemDuration->setZValue(1);
    nodeItemDuration->setBounds(QRectF(rectDuration->rect().x(), rectDuration->rect().center().y(), rectDuration->rect().width(), 0));
    nodeItemDuration->setBoundsScaled(QRectF(0.05, 0, 20 - 0.05, 0));
    nodeItemDuration->setXScaled(client->getEnvelope()->getDuration());
    QObject::connect(nodeItemDuration, SIGNAL(xChangedScaled(qreal)), this, SLOT(onDurationNodePositionChanged(qreal)));

    nodeItemSustainPosition = new GraphicsNodeItem(-5.0, -5.0, 10.0, 10.0, this);
    nodeItemSustainPosition->setPen(nodePen);
    nodeItemSustainPosition->setBrush(nodeBrush);
    nodeItemSustainPosition->setZValue(1);
    nodeItemSustainPosition->setBounds(QRectF(rectSustain->rect().x(), rectSustain->rect().center().y(), rectSustain->rect().width(), 0));
    nodeItemSustainPosition->setBoundsScaled(QRectF(0, 0, client->getEnvelope()->getDuration(), 0));
    nodeItemSustainPosition->setXScaled(client->getEnvelope()->getSustainPosition());
    QObject::connect(nodeItemSustainPosition, SIGNAL(xChangedScaled(qreal)), this, SLOT(onSustainNodePositionChanged(qreal)));

    sustainPositionLine = new QGraphicsLineItem(0, 0, 0, interpolatorEditItem->rect().height(), this);
    sustainPositionLine->setPen(QPen(Qt::DotLine));
    sustainPositionLine->setPos(nodeItemSustainPosition->x(), interpolatorEditItem->rect().y());

    sustainPositionText = new QGraphicsSimpleTextItem(QString("Sustain: %1s").arg(client->getEnvelope()->getSustainPosition()), this);
    sustainPositionText->setPos(nodeItemSustainPosition->x() + 8, nodeItemSustainPosition->y() - 8);

    durationText = new QGraphicsSimpleTextItem(QString("Duration: %1s").arg(client->getEnvelope()->getDuration()), this);
    durationText->setPos(nodeItemDuration->x() + 8, nodeItemDuration->y() - 8);
}

EnvelopeClient * EnvelopeGraphicsItem::getClient()
{
    return client;
}

void EnvelopeGraphicsItem::onDurationNodePositionChanged(qreal x)
{
    client->postChangeDuration(x);
    // update the graphics:
    interpolatorEditItem->setRect(QRectF(rect().x(), rect().y(), rect().width(), rect().height() - 32), QRectF(0, 1, client->getEnvelope()->getDuration(), -2));
    durationText->setText(QString("Duration: %1s").arg(client->getEnvelope()->getDuration()));
    durationText->setPos(nodeItemDuration->x() + 8, nodeItemDuration->y() - 8);
    nodeItemSustainPosition->setBoundsScaled(QRectF(0, 0, client->getEnvelope()->getDuration(), 0));
    nodeItemSustainPosition->setXScaled(client->getEnvelope()->getSustainPosition());
    sustainPositionLine->setPos(nodeItemSustainPosition->x(), interpolatorEditItem->rect().y());
    sustainPositionText->setText(QString("Sustain: %1 sec.").arg(client->getEnvelope()->getSustainPosition()));
    sustainPositionText->setPos(nodeItemSustainPosition->x() + 8, nodeItemSustainPosition->y() - 8);
}

void EnvelopeGraphicsItem::onSustainNodePositionChanged(qreal x)
{
    client->postChangeSustainPosition(x);
    // update the graphics:
    sustainPositionLine->setPos(nodeItemSustainPosition->x(), interpolatorEditItem->rect().y());
    sustainPositionText->setText(QString("Sustain: %1 sec.").arg(client->getEnvelope()->getSustainPosition()));
    sustainPositionText->setPos(nodeItemSustainPosition->x() + 8, nodeItemSustainPosition->y() - 8);
}

class EnvelopeClientFactory : public JackClientFactory
{
public:
    EnvelopeClientFactory()
    {
        JackClientFactory::registerFactory(this);
    }
    QString getName()
    {
        return "Envelope";
    }
    JackClient * createClient(const QString &clientName)
    {
        return new EnvelopeClient(clientName);
    }
    static EnvelopeClientFactory factory;
};

EnvelopeClientFactory EnvelopeClientFactory::factory;

JackClientFactory * EnvelopeClient::getFactory()
{
    return &EnvelopeClientFactory::factory;
}
