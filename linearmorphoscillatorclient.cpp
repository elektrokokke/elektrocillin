#include "linearmorphoscillatorclient.h"
#include <cmath>
#include <QPen>

LinearMorphOscillatorClient::LinearMorphOscillatorClient(const QString &clientName, size_t ringBufferSize) :
    EventProcessorClient<LinearMorphOscillatorParameters>(clientName, new LinearMorphOscillator(LinearInterpolator(), LinearInterpolator()), ringBufferSize)
{
    state[0].getX().append(0);
    state[0].getY().append(-1);
    state[0].getX().append(1.9 * M_PI);
    state[0].getY().append(1);
    state[0].getX().append(2.0 * M_PI);
    state[0].getY().append(-1);
    getLinearMorphOscillator()->setState(0, state[0]);
    state[1] = state[0];
    state[1].getX()[1] = 0.1 * M_PI;
    getLinearMorphOscillator()->setState(1, state[1]);
}

LinearMorphOscillatorClient::~LinearMorphOscillatorClient()
{
    close();
}

LinearMorphOscillator * LinearMorphOscillatorClient::getLinearMorphOscillator()
{
    return (LinearMorphOscillator*)getMidiProcessor();
}

LinearInterpolator * LinearMorphOscillatorClient::getState(int stateIndex)
{
    Q_ASSERT((stateIndex >= 0) && (stateIndex < 2));
    return &state[stateIndex];
}

void LinearMorphOscillatorClient::postIncreaseControlPoints()
{
    int size = state[0].getX().size() + 1;
    QVector<LinearMorphOscillatorParameters> parameterVector;
    for (int stateIndex = 0; stateIndex < 2; stateIndex++) {
        double stretchFactor = (double)(state[stateIndex].getX().size() - 1) / (double)(size - 1);
        state[stateIndex].getX().append(2 * M_PI);
        state[stateIndex].getY().append(1);
        for (int i = size - 1; i >= 0; i--) {
            if (i < size - 1) {
                state[stateIndex].getX()[i] = state[stateIndex].getX()[i] * stretchFactor;
            }
            LinearMorphOscillatorParameters parameters;
            parameters.state = stateIndex;
            parameters.controlPoints = size;
            parameters.index = i;
            parameters.x = state[stateIndex].getX()[i];
            parameters.y = state[stateIndex].getY()[i];
            parameterVector.append(parameters);
        }
    }
    postEvents(parameterVector);
}

void LinearMorphOscillatorClient::postDecreaseControlPoints()
{
    if (state[0].getX().size() > 2) {
        QVector<LinearMorphOscillatorParameters> parameterVector;
        int size = state[0].getX().size() - 1;
        for (int stateIndex = 0; stateIndex < 2; stateIndex++) {
            state[stateIndex].getX().resize(size);
            state[stateIndex].getY().resize(size);
            double stretchFactor = 2 * M_PI / state[stateIndex].getX().back();
            for (int i = size - 1; i >= 0; i--) {
                state[stateIndex].getX()[i] = state[stateIndex].getX()[i] * stretchFactor;
                LinearMorphOscillatorParameters parameters;
                parameters.state = stateIndex;
                parameters.controlPoints = size;
                parameters.index = i;
                parameters.x = state[stateIndex].getX()[i];
                parameters.y = state[stateIndex].getY()[i];
                parameterVector.append(parameters);
            }
        }
        postEvents(parameterVector);
    }
}

void LinearMorphOscillatorClient::postChangeControlPoint(int stateIndex, int index, int nrOfControlPoints, double x, double y)
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
    LinearMorphOscillatorParameters parameters;
    parameters.state = stateIndex;
    parameters.controlPoints = nrOfControlPoints;
    parameters.index = index;
    state[stateIndex].getX()[index] = parameters.x = x;
    state[stateIndex].getY()[index] = parameters.y = y;
    postEvent(parameters);
}

void LinearMorphOscillatorClient::processEvent(const LinearMorphOscillatorParameters &event, jack_nframes_t time)
{
    getLinearMorphOscillator()->processEvent(event, time);
}

LinearMorphOscillatorGraphicsSubItem::LinearMorphOscillatorGraphicsSubItem(const QRectF &rect, LinearMorphOscillatorClient *client_, int state_, QGraphicsItem *parent) :
    GraphicsInterpolatorEditItem(client_->getState(state_), rect, QRectF(0, 1, 2.0 * M_PI, -2), parent),
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

void LinearMorphOscillatorGraphicsSubItem::changeControlPoint(int index, int nrOfControlPoints, double x, double y)
{
    client->postChangeControlPoint(state, index, nrOfControlPoints, x, y);
}

LinearMorphOscillatorGraphicsItem::LinearMorphOscillatorGraphicsItem(const QRectF &rect, LinearMorphOscillatorClient *client, QGraphicsItem *parent) :
    QGraphicsRectItem(rect, parent)
{
    setPen(QPen(Qt::NoPen));
    LinearMorphOscillatorGraphicsSubItem *state1 = new LinearMorphOscillatorGraphicsSubItem(QRect(rect.x(), rect.y(), rect.width() * 0.49, rect.height()), client, 0, this);
    LinearMorphOscillatorGraphicsSubItem *state2 = new LinearMorphOscillatorGraphicsSubItem(QRect(rect.x() + rect.width() * 0.51, rect.y(), rect.width() * 0.49, rect.height()), client, 1, this);
    state1->setTwin(state2);
    state2->setTwin(state1);
}
