/*
    Copyright 2011 Arne Jacobs

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "envelopeclient.h"
#include <cmath>

EnvelopeClient::EnvelopeClient(const QString &clientName, Envelope *envelope_, size_t ringBufferSize) :
    EventProcessorClient(clientName, envelope_, envelope_, envelope_, ringBufferSize),
    envelopeProcess(envelope_)
{
    envelope = *envelopeProcess;
}

EnvelopeClient::~EnvelopeClient()
{
    close();
    delete envelopeProcess;
}

void EnvelopeClient::saveState(QDataStream &stream)
{
    EventProcessorClient::saveState(stream);
    envelope.save(stream);
}

void EnvelopeClient::loadState(QDataStream &stream)
{
    EventProcessorClient::loadState(stream);
    envelopeProcess->load(stream);
    envelope = *envelopeProcess;
}

Envelope * EnvelopeClient::getEnvelope()
{
    return &envelope;
}

void EnvelopeClient::postIncreaseControlPoints()
{
    InterpolatorProcessor::AddControlPointsEvent *event = new InterpolatorProcessor::AddControlPointsEvent(true, false, false, true);
    envelope.processEvent(event, 0);
    postEvent(event);
}

void EnvelopeClient::postDecreaseControlPoints()
{
    if (envelope.getInterpolator()->getX().size() > 2) {
        InterpolatorProcessor::DeleteControlPointsEvent *event = new InterpolatorProcessor::DeleteControlPointsEvent(true, false, false, true);
        envelope.processEvent(event, 0);
        postEvent(event);
    }
}

void EnvelopeClient::postChangeControlPoint(int index, double x, double y)
{
    InterpolatorProcessor::ChangeControlPointEvent *event = new InterpolatorProcessor::ChangeControlPointEvent(index, x, y);
    envelope.processEvent(event, 0);
    postEvent(event);
}

void EnvelopeClient::postChangeSustainIndex(int sustainIndex)
{
    Envelope::ChangeSustainPositionEvent *event = new Envelope::ChangeSustainPositionEvent(sustainIndex);
    envelope.processEvent(event, 0);
    postEvent(event);
}

QGraphicsItem * EnvelopeClient::createGraphicsItem()
{
    return new EnvelopeGraphicsItem(QRectF(0, 0, 1200, 420), this);
}

EnvelopeGraphicsItem::EnvelopeGraphicsItem(const QRectF &rect, EnvelopeClient *client_, QGraphicsItem *parent_) :
    GraphicsInterpolatorEditItem(client_->getEnvelope()->getInterpolator(),
        rect,
        QRectF(0, 1, log(client_->getEnvelope()->getDurationInSeconds() + 1), -2),
        parent_,
        4,
        8,
        true
        ),
    client(client_)
{
    setVisible(GraphicsInterpolatorEditItem::FIRST, false);
    setCursor(Qt::ArrowCursor);
    // create a child that allows selection of the sustain node:
    sustainNodeControlItem = new GraphicsDiscreteControlItem("Sustain node", 1, client->getEnvelope()->getInterpolator()->getX().size() - 1, client->getEnvelope()->getSustainIndex(), 100 + (client->getEnvelope()->getInterpolator()->getX().size() - 1) * 10, GraphicsContinuousControlItem::HORIZONTAL, this);
    sustainNodeControlItem->setPos(getInnerRectangle().topRight() - QPointF(sustainNodeControlItem->rect().width(), 0));
    QObject::connect(sustainNodeControlItem, SIGNAL(valueChanged(int)), this, SLOT(onSustainNodeChanged(int)));
}

EnvelopeClient * EnvelopeGraphicsItem::getClient()
{
    return client;
}

void EnvelopeGraphicsItem::increaseControlPoints()
{
    client->postIncreaseControlPoints();
    sustainNodeControlItem->setValue(client->getEnvelope()->getSustainIndex());
    sustainNodeControlItem->setMaxValue(client->getEnvelope()->getInterpolator()->getX().size() - 1);
    sustainNodeControlItem->setSize(100 + (client->getEnvelope()->getInterpolator()->getX().size() - 1) * 10);
}

void EnvelopeGraphicsItem::decreaseControlPoints()
{
    client->postDecreaseControlPoints();
    sustainNodeControlItem->setValue(client->getEnvelope()->getSustainIndex());
    sustainNodeControlItem->setMaxValue(client->getEnvelope()->getInterpolator()->getX().size() - 1);
    sustainNodeControlItem->setSize(100 + (client->getEnvelope()->getInterpolator()->getX().size() - 1) * 10);
}

void EnvelopeGraphicsItem::changeControlPoint(int index, double x, double y)
{
    client->postChangeControlPoint(index, x, y);
}

void EnvelopeGraphicsItem::onSustainNodeChanged(int value)
{
    client->postChangeSustainIndex(value);
    interpolatorChanged();
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
        return new EnvelopeClient(clientName, new Envelope());
    }
    static EnvelopeClientFactory factory;
};

EnvelopeClientFactory EnvelopeClientFactory::factory;

JackClientFactory * EnvelopeClient::getFactory()
{
    return &EnvelopeClientFactory::factory;
}
