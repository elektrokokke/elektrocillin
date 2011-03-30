#include "linearmorphoscillatorclient.h"
#include <cmath>
#include <QPen>

LinearMorphOscillatorClient::LinearMorphOscillatorClient(const QString &clientName, double frequencyModulationIntensity, size_t ringBufferSize) :
    OscillatorClient(clientName, new LinearMorphOscillator(LinearInterpolator(), LinearInterpolator(), frequencyModulationIntensity), ringBufferSize)
{
    QVector<double> xx, yy;
    xx.append(0);
    yy.append(-1);
    xx.append(1);
    yy.append(1);
    xx.append(1);
    yy.append(-1);
    state[0].changeControlPoints(xx, yy);
    xx[1] = 0;
    state[1].changeControlPoints(xx, yy);
    for (int i = 0; i < 2; i++) {
        getLinearMorphOscillator()->setState(i, state[i]);
    }
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
    InterpolatorProcessor::AddControlPointsEvent *event = new InterpolatorProcessor::AddControlPointsEvent(true, false, false, true);
    state[0].addControlPoints(true, false, false, true);
    state[1].addControlPoints(true, false, false, true);
    postEvent(event);
}

void LinearMorphOscillatorClient::postDecreaseControlPoints()
{
    if (state[0].getX().size() > 2) {
        InterpolatorProcessor::DeleteControlPointsEvent *event = new InterpolatorProcessor::DeleteControlPointsEvent(true, false, false, true);
        state[0].deleteControlPoints(true, false, false, true);
        state[1].deleteControlPoints(true, false, false, true);
        postEvent(event);
    }
}

void LinearMorphOscillatorClient::postChangeControlPoint(int stateIndex, int index, double x, double y)
{
    Q_ASSERT((stateIndex >= 0) && (stateIndex < 2));
    LinearMorphOscillator::ChangeControlPointEvent *event = new LinearMorphOscillator::ChangeControlPointEvent(stateIndex, index, x, y);
    state[stateIndex].changeControlPoint(index, x, y);
    postEvent(event);
}

QGraphicsItem * LinearMorphOscillatorClient::createGraphicsItem()
{
    int padding = 4;
    QGraphicsRectItem *item = new QGraphicsRectItem();
    QGraphicsItem *oscillatorItem = OscillatorClient::createGraphicsItem();
    QRectF rect = QRect(0, 0, 600, 420);
    rect = rect.translated(oscillatorItem->boundingRect().width() + 2 * padding, padding);
    oscillatorItem->setPos(padding, padding);
    oscillatorItem->setParentItem(item);
    QGraphicsItem *ourItem = new LinearMorphOscillatorGraphicsItem(rect, this);
    ourItem->setParentItem(item);
    item->setRect((rect | oscillatorItem->boundingRect().translated(oscillatorItem->pos())).adjusted(-padding, -padding, padding, padding));
    item->setPen(QPen(QBrush(Qt::black), 1));
    item->setBrush(QBrush(Qt::white));
    return item;
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
    int padding = 4;
    setPen(QPen(Qt::NoPen));
    LinearMorphOscillatorGraphicsSubItem *state2 = new LinearMorphOscillatorGraphicsSubItem(QRect(rect.x() + rect.width() * 0.5 + 0.5 * padding, rect.y(), rect.width() * 0.5 - 0.5 * padding, rect.height()), client, 1, this);
    state2->getGraphItem()->setNodePen(QPen(QBrush(Qt::green), 3));
    state2->getGraphItem()->setNodeBrush(QBrush(Qt::darkGreen));
    LinearMorphOscillatorGraphicsSubItem *state1 = new LinearMorphOscillatorGraphicsSubItem(QRect(rect.x(), rect.y(), rect.width() * 0.5 - 0.5 * padding, rect.height()), client, 0, this);
    state1->getGraphItem()->setNodePen(QPen(QBrush(Qt::red), 3));
    state1->getGraphItem()->setNodeBrush(QBrush(Qt::darkRed));
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
