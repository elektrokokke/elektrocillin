#include "envelopeclient.h"
#include <cmath>

EnvelopeClient::EnvelopeClient(const QString &clientName, size_t ringBufferSize) :
    EventProcessorClient(clientName, new Envelope(), ringBufferSize)
{
    envelopeProcess = (Envelope*)getAudioProcessor();
    envelope = new Envelope();
    envelope->copyInterpolator(envelopeProcess);
}

EnvelopeClient::~EnvelopeClient()
{
    close();
    delete envelope;
    delete envelopeProcess;
}

void EnvelopeClient::saveState(QDataStream &stream)
{
    envelope->save(stream);
}

void EnvelopeClient::loadState(QDataStream &stream)
{
    envelopeProcess->load(stream);
    envelope->copyInterpolator(envelopeProcess);
}

Envelope * EnvelopeClient::getEnvelope()
{
    return envelope;
}

void EnvelopeClient::postIncreaseControlPoints()
{
    Interpolator::ChangeAllControlPointsEvent *event = envelope->createIncreaseControlPointsEvent();
    envelope->processEvent(event, 0);
    postEvent(event);
}

void EnvelopeClient::postDecreaseControlPoints()
{
    if (envelope->getInterpolator()->getX().size() > 2) {
        Interpolator::ChangeAllControlPointsEvent *event = envelope->createDecreaseControlPointsEvent();
        envelope->processEvent(event, 0);
        postEvent(event);
    }
}

void EnvelopeClient::postChangeControlPoint(int index, double x, double y)
{
    Interpolator *interpolator = envelope->getInterpolator();
    if (index == 0) {
       x = interpolator->getX()[0];
    }
    if (index == interpolator->getX().size() - 1) {
        y = 0;
    }
    if ((index > 0) && (x <= interpolator->getX()[index - 1])) {
        x = interpolator->getX()[index - 1];
    }
    if ((index < interpolator->getX().size() - 1) && (x >= interpolator->getX()[index + 1])) {
        x = interpolator->getX()[index + 1];
    }
    Interpolator::ChangeControlPointEvent *event = new Interpolator::ChangeControlPointEvent();
    event->index = index;
    event->x = x;
    event->y = y;
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

QGraphicsItem * EnvelopeClient::createGraphicsItem()
{
    return new EnvelopeGraphicsItem(QRectF(0, 0, 1200, 420), this);
}

void EnvelopeClient::processEvent(const RingBufferEvent *event, jack_nframes_t time)
{
    if (const Interpolator::ChangeControlPointEvent *changeControlPointEvent = dynamic_cast<const Interpolator::ChangeControlPointEvent*>(event)) {
        envelopeProcess->processEvent(changeControlPointEvent, time);
    } else if (const Interpolator::ChangeAllControlPointsEvent *changeAllControlPointsEvent = dynamic_cast<const Interpolator::ChangeAllControlPointsEvent*>(event)) {
        envelopeProcess->processEvent(changeAllControlPointsEvent, time);
    } else if (const Envelope::ChangeSustainPositionEvent *changeSustainPositionEvent = dynamic_cast<const Envelope::ChangeSustainPositionEvent*>(event)) {
        envelopeProcess->processEvent(changeSustainPositionEvent, time);
    }
}

EnvelopeGraphicsSubItem::EnvelopeGraphicsSubItem(const QRectF &rect, EnvelopeGraphicsItem *parent_, const QPen &nodePen, const QBrush &nodeBrush) :
    GraphicsInterpolatorEditItem(parent_->getClient()->getEnvelope()->getInterpolator(),
        rect,
        QRectF(0, 1, log(parent_->getClient()->getEnvelope()->getDurationInSeconds() + 1), -2),
        parent_,
        4,
        8,
        true,
        nodePen,
        nodeBrush
        ),
    parent(parent_)
{
    setVisible(GraphicsInterpolatorEditItem::FIRST, false);
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
    font.setPointSize(8);

    interpolatorEditItem = new EnvelopeGraphicsSubItem(QRectF(rect.x(), rect.y(), rect.width(), rect.height() - 16), this, nodePen, nodeBrush);
    interpolatorEditItem->setBrush(QBrush(Qt::NoBrush));

    QRectF innerRect = interpolatorEditItem->getInnerRectangle();
    QGraphicsRectItem *rectSustain = new QGraphicsRectItem(innerRect.left(), rect.bottom() - 16, innerRect.width(), 16, this);
    rectSustain->setPen(QPen(QBrush(Qt::black), 2));

    nodeItemSustainPosition = new GraphicsNodeItem(-5.0, -5.0, 10.0, 10.0, this);
    nodeItemSustainPosition->setPen(nodePen);
    nodeItemSustainPosition->setBrush(nodeBrush);
    nodeItemSustainPosition->setZValue(1);
    nodeItemSustainPosition->setBounds(QRectF(rectSustain->rect().x(), rectSustain->rect().center().y(), rectSustain->rect().width(), 0));
    nodeItemSustainPosition->setBoundsScaled(QRectF(0, 0, log(client->getEnvelope()->getDurationInSeconds() + 1), 0));
    nodeItemSustainPosition->setXScaled(client->getEnvelope()->getSustainPosition());
    QObject::connect(nodeItemSustainPosition, SIGNAL(xChangedScaled(qreal)), this, SLOT(onSustainNodePositionChanged(qreal)));

    sustainPositionLine = new QGraphicsLineItem(0, 0, 0, interpolatorEditItem->rect().height(), this);
    sustainPositionLine->setPen(QPen(Qt::DotLine));
    sustainPositionLine->setPos(nodeItemSustainPosition->x(), interpolatorEditItem->rect().y());

    sustainPositionText = new QGraphicsSimpleTextItem(QString("Sustain: %1s").arg(client->getEnvelope()->getSustainPositionInSeconds()), this);
    sustainPositionText->setFont(font);
    sustainPositionText->setPos(nodeItemSustainPosition->x() + 8, nodeItemSustainPosition->y() - sustainPositionText->boundingRect().height() * 0.3);
}

EnvelopeClient * EnvelopeGraphicsItem::getClient()
{
    return client;
}

void EnvelopeGraphicsItem::onSustainNodePositionChanged(qreal x)
{
    client->postChangeSustainPosition(x);
    // update the graphics:
    sustainPositionLine->setPos(nodeItemSustainPosition->x(), interpolatorEditItem->rect().y());
    sustainPositionText->setText(QString("Sustain: %1 sec.").arg(client->getEnvelope()->getSustainPositionInSeconds()));
    sustainPositionText->setPos(nodeItemSustainPosition->x() + 8, nodeItemSustainPosition->y() - sustainPositionText->boundingRect().height() * 0.3);
}

class EnvelopeClientFactory : public JackClientFactory
{
public:
    EnvelopeClientFactory()
    {
        JackClientSerializer::getInstance()->registerFactory(this);
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
