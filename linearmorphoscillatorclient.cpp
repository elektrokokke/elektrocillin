#include "linearmorphoscillatorclient.h"
#include <cmath>
#include <QPen>

LinearMorphOscillatorClient::LinearMorphOscillatorClient(const QString &clientName, double frequencyModulationIntensity, size_t ringBufferSize) :
    OscillatorClient(clientName, new LinearMorphOscillator(LinearInterpolator(), LinearInterpolator(), frequencyModulationIntensity), ringBufferSize)
{
    state[0].getX().append(0);
    state[0].getY().append(-1);
    state[0].getX().append(0.95);
    state[0].getY().append(1);
    state[0].getX().append(1);
    state[0].getY().append(-1);
    getLinearMorphOscillator()->setState(0, state[0]);
    state[1] = state[0];
    state[1].getX()[1] = 0.05;
    getLinearMorphOscillator()->setState(1, state[1]);
}

LinearMorphOscillatorClient::~LinearMorphOscillatorClient()
{
    close();
}

void LinearMorphOscillatorClient::saveState(QDataStream &stream)
{
    OscillatorClient::saveState(stream);
    state[0].save(stream);
    state[1].save(stream);
}

void LinearMorphOscillatorClient::loadState(QDataStream &stream)
{
    OscillatorClient::loadState(stream);
    state[0].load(stream);
    state[1].load(stream);
    getLinearMorphOscillator()->setState(0, state[0]);
    getLinearMorphOscillator()->setState(1, state[1]);
}

LinearMorphOscillator * LinearMorphOscillatorClient::getLinearMorphOscillator()
{
    return (LinearMorphOscillator*)getAudioProcessor();
}

LinearInterpolator * LinearMorphOscillatorClient::getState(int stateIndex)
{
    Q_ASSERT((stateIndex >= 0) && (stateIndex < 2));
    return &state[stateIndex];
}

void LinearMorphOscillatorClient::postIncreaseControlPoints()
{
    int size = state[0].getX().size() + 1;
    QVector<RingBufferEvent*> events;
    for (int stateIndex = 0; stateIndex < 2; stateIndex++) {
        double stretchFactor = (double)(state[stateIndex].getX().size() - 1) / (double)(size - 1);
        state[stateIndex].getX().append(1);
        state[stateIndex].getY().append(1);
        for (int i = size - 1; i >= 0; i--) {
            if (i < size - 1) {
                state[stateIndex].getX()[i] = state[stateIndex].getX()[i] * stretchFactor;
            }
        }
        LinearMorphOscillator::ChangeAllControlPointsEvent *event = new LinearMorphOscillator::ChangeAllControlPointsEvent();
        event->state = stateIndex;
        event->xx = state[stateIndex].getX();
        event->yy = state[stateIndex].getY();
        events.append(event);
    }
    postEvents(events);
}

void LinearMorphOscillatorClient::postDecreaseControlPoints()
{
    if (state[0].getX().size() > 2) {
        QVector<RingBufferEvent*> events;
        int size = state[0].getX().size() - 1;
        for (int stateIndex = 0; stateIndex < 2; stateIndex++) {
            state[stateIndex].getX().resize(size);
            state[stateIndex].getY().resize(size);
            double stretchFactor = 1.0 / state[stateIndex].getX().back();
            for (int i = size - 1; i >= 0; i--) {
                state[stateIndex].getX()[i] = state[stateIndex].getX()[i] * stretchFactor;
            }
            LinearMorphOscillator::ChangeAllControlPointsEvent *event = new LinearMorphOscillator::ChangeAllControlPointsEvent();
            event->state = stateIndex;
            event->xx = state[stateIndex].getX();
            event->yy = state[stateIndex].getY();
            events.append(event);
        }
        postEvents(events);
    }
}

void LinearMorphOscillatorClient::postChangeControlPoint(int stateIndex, int index, double x, double y)
{
    Q_ASSERT((stateIndex >= 0) && (stateIndex < 2));
    if (index == 0) {
       x = state[stateIndex].getX()[0];
    }
    if (index == state[stateIndex].getX().size() - 1) {
        x = state[stateIndex].getX().back();
    }
    if ((index > 0) && (x <= state[stateIndex].getX()[index - 1])) {
        x = state[stateIndex].getX()[index - 1];
    }
    if ((index < state[stateIndex].getX().size() - 1) && (x >= state[stateIndex].getX()[index + 1])) {
        x = state[stateIndex].getX()[index + 1];
    }
    LinearMorphOscillator::ChangeControlPointEvent *event = new LinearMorphOscillator::ChangeControlPointEvent();
    event->state = stateIndex;
    event->index = index;
    state[stateIndex].getX()[index] = event->x = x;
    state[stateIndex].getY()[index] = event->y = y;
    postEvent(event);
}

QGraphicsItem * LinearMorphOscillatorClient::createGraphicsItem()
{
    QRectF rect(0, 0, 600, 420);
    QGraphicsRectItem *rectItem = new QGraphicsRectItem(rect);
    rectItem->setPen(QPen(Qt::NoPen));
    QRectF rectGain(rect.x(), rect.y(), 16, rect.height());
    QRectF rectOscillator = rect.adjusted(rectGain.width(), 0, 0, 0);
    (new OscillatorClientGraphicsItem(rectGain, this))->setParentItem(rectItem);
    (new LinearMorphOscillatorGraphicsItem(rectOscillator, this))->setParentItem(rectItem);
    return rectItem;
}

void LinearMorphOscillatorClient::processEvent(const RingBufferEvent *event, jack_nframes_t time)
{
    if (const LinearMorphOscillator::ChangeControlPointEvent *changeControlPointEvent = dynamic_cast<const LinearMorphOscillator::ChangeControlPointEvent*>(event)) {
        getLinearMorphOscillator()->processEvent(changeControlPointEvent, time);
    } else if (const LinearMorphOscillator::ChangeAllControlPointsEvent *changeAllControlPointsEvent = dynamic_cast<const LinearMorphOscillator::ChangeAllControlPointsEvent*>(event)) {
        getLinearMorphOscillator()->processEvent(changeAllControlPointsEvent, time);
    } else {
        OscillatorClient::processEvent(event, time);
    }
}

LinearMorphOscillatorGraphicsSubItem::LinearMorphOscillatorGraphicsSubItem(const QRectF &rect, LinearMorphOscillatorClient *client_, int state_, QGraphicsItem *parent) :
    GraphicsInterpolatorEditItem(client_->getState(state_),
        rect,
        QRectF(client_->getState(state_)->getX().first(), 1, client_->getState(state_)->getX().last() - client_->getState(state_)->getX().first(), -2),
        parent,
        8,
        8,
        false
        ),
    client(client_),
    state(state_),
    twin(0)
{}

void LinearMorphOscillatorGraphicsSubItem::setTwin(LinearMorphOscillatorGraphicsSubItem *twin)
{
    this->twin = twin;
}

void LinearMorphOscillatorGraphicsSubItem::increaseControlPoints()
{
    client->postIncreaseControlPoints();
    if (twin) {
        twin->interpolatorChanged();
    }
}

void LinearMorphOscillatorGraphicsSubItem::decreaseControlPoints()
{
    client->postDecreaseControlPoints();
    if (twin) {
        twin->interpolatorChanged();
    }
}

void LinearMorphOscillatorGraphicsSubItem::changeControlPoint(int index, double x, double y)
{
    client->postChangeControlPoint(state, index, x, y);
}

LinearMorphOscillatorGraphicsItem::LinearMorphOscillatorGraphicsItem(const QRectF &rect, LinearMorphOscillatorClient *client, QGraphicsItem *parent) :
    QGraphicsRectItem(rect, parent)
{
    setPen(QPen(Qt::NoPen));
    LinearMorphOscillatorGraphicsSubItem *state1 = new LinearMorphOscillatorGraphicsSubItem(QRect(rect.x(), rect.y(), rect.width() * 0.49, rect.height()), client, 0, this);
    state1->setPen(QPen(QBrush(Qt::red), 3));
    state1->setBrush(QBrush(Qt::darkRed));
    LinearMorphOscillatorGraphicsSubItem *state2 = new LinearMorphOscillatorGraphicsSubItem(QRect(rect.x() + rect.width() * 0.51, rect.y(), rect.width() * 0.49, rect.height()), client, 1, this);
    state2->setPen(QPen(QBrush(Qt::green), 3));
    state2->setBrush(QBrush(Qt::darkGreen));
    state1->setTwin(state2);
    state2->setTwin(state1);
}

class LinearMorphOscillatorClientFactory : public JackClientFactory
{
public:
    LinearMorphOscillatorClientFactory()
    {
        JackClientSerializer::getInstance()->registerFactory(this);
    }
    QString getName()
    {
        return "Morph oscillator";
    }
    JackClient * createClient(const QString &clientName)
    {
        return new LinearMorphOscillatorClient(clientName);
    }
    static LinearMorphOscillatorClientFactory factory;
};

LinearMorphOscillatorClientFactory LinearMorphOscillatorClientFactory::factory;

JackClientFactory * LinearMorphOscillatorClient::getFactory()
{
    return &LinearMorphOscillatorClientFactory::factory;
}
