/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

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

EnvelopeClient::EnvelopeClient(const QString &clientName, Envelope *processEnvelope_, Envelope *guiEnvelope_, size_t ringBufferSize) :
    ParameterClient(clientName, processEnvelope_, processEnvelope_, processEnvelope_, processEnvelope_, guiEnvelope_, ringBufferSize),
    processEnvelope(processEnvelope_),
    guiEnvelope(guiEnvelope_)
{
}

EnvelopeClient::~EnvelopeClient()
{
    close();
    delete processEnvelope;
    delete guiEnvelope;
}

void EnvelopeClient::saveState(QDataStream &stream)
{
    EventProcessorClient::saveState(stream);
    guiEnvelope->save(stream);
}

void EnvelopeClient::loadState(QDataStream &stream)
{
    EventProcessorClient::loadState(stream);
    guiEnvelope->load(stream);
    *processEnvelope = *guiEnvelope;
}

Envelope * EnvelopeClient::getEnvelope()
{
    return guiEnvelope;
}

void EnvelopeClient::postIncreaseControlPoints()
{
    InterpolatorProcessor::AddControlPointsEvent *event = new InterpolatorProcessor::AddControlPointsEvent(true, false, false, true);
    guiEnvelope->processEvent(event, 0);
    postEvent(event);
}

void EnvelopeClient::postDecreaseControlPoints()
{
    if (guiEnvelope->getX().size() > 3) {
        InterpolatorProcessor::DeleteControlPointsEvent *event = new InterpolatorProcessor::DeleteControlPointsEvent(true, false, false, true);
        guiEnvelope->processEvent(event, 0);
        postEvent(event);
    }
}

void EnvelopeClient::postChangeControlPoint(int index, double x, double y)
{
    InterpolatorProcessor::ChangeControlPointEvent *event = new InterpolatorProcessor::ChangeControlPointEvent(index, x, y);
    guiEnvelope->processEvent(event, 0);
    postEvent(event);
}

void EnvelopeClient::postChangeSustainIndex(int sustainIndex)
{
    Envelope::ChangeSustainPositionEvent *event = new Envelope::ChangeSustainPositionEvent(sustainIndex);
    guiEnvelope->processEvent(event, 0);
    postEvent(event);
}

QGraphicsItem * EnvelopeClient::createGraphicsItem()
{
    int padding = 4;
    QGraphicsRectItem *item = new QGraphicsRectItem();
    QGraphicsItem *parameterItem = ParameterClient::createGraphicsItem();
    QRectF rect = QRect(0, 0, 1200, 420);
    rect = rect.translated(parameterItem->boundingRect().width() + 2 * padding, padding);
    parameterItem->setPos(padding, padding);
    parameterItem->setParentItem(item);
    QGraphicsItem *ourItem = new EnvelopeGraphicsItem(rect, this);
    ourItem->setParentItem(item);
    item->setRect((rect | parameterItem->boundingRect().translated(parameterItem->pos())).adjusted(-padding, -padding, padding, padding));
    item->setPen(QPen(QBrush(Qt::black), 1));
    item->setBrush(QBrush(Qt::white));
    return item;
}

EnvelopeGraphicsItem::EnvelopeGraphicsItem(const QRectF &rect, EnvelopeClient *client_, QGraphicsItem *parent_) :
    GraphicsInterpolatorEditItem(client_->getEnvelope(),
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
    sustainNodeControlItem = new GraphicsDiscreteControlItem("Sustain node", 1, client->getEnvelope()->getX().size() - 1, client->getEnvelope()->getSustainIndex(), 100 + (client->getEnvelope()->getX().size() - 1) * 10, GraphicsContinuousControlItem::HORIZONTAL, this);
    sustainNodeControlItem->setPos(getInnerRectangle().topRight() - QPointF(sustainNodeControlItem->rect().width(), 0));
    QObject::connect(sustainNodeControlItem, SIGNAL(valueChanged(int)), this, SLOT(onSustainNodeChanged(int)));
    QObject::connect(client, SIGNAL(changedParameters()), this, SLOT(updateInterpolator()));
}

EnvelopeClient * EnvelopeGraphicsItem::getClient()
{
    return client;
}

void EnvelopeGraphicsItem::increaseControlPoints()
{
    client->postIncreaseControlPoints();
    sustainNodeControlItem->setValue(client->getEnvelope()->getSustainIndex(), false);
    sustainNodeControlItem->setMaxValue(client->getEnvelope()->getX().size() - 1);
    sustainNodeControlItem->setSize(100 + (client->getEnvelope()->getX().size() - 1) * 10);
}

void EnvelopeGraphicsItem::decreaseControlPoints()
{
    client->postDecreaseControlPoints();
    sustainNodeControlItem->setValue(client->getEnvelope()->getSustainIndex(), false);
    sustainNodeControlItem->setMaxValue(client->getEnvelope()->getX().size() - 1);
    sustainNodeControlItem->setSize(100 + (client->getEnvelope()->getX().size() - 1) * 10);
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

void EnvelopeGraphicsItem::updateInterpolator()
{
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
        return new EnvelopeClient(clientName, new Envelope(), new Envelope());
    }
    static EnvelopeClientFactory factory;
};

EnvelopeClientFactory EnvelopeClientFactory::factory;

JackClientFactory * EnvelopeClient::getFactory()
{
    return &EnvelopeClientFactory::factory;
}
