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
    activateMidiOutput(false);
}

EnvelopeClient::~EnvelopeClient()
{
    close();
    delete processEnvelope;
    delete guiEnvelope;
}

void EnvelopeClient::saveState(QDataStream &stream)
{
    ParameterClient::saveState(stream);
    guiEnvelope->save(stream);
}

void EnvelopeClient::loadState(QDataStream &stream)
{
    ParameterClient::loadState(stream);
    guiEnvelope->load(stream);
    *processEnvelope = *guiEnvelope;
}

QGraphicsItem * EnvelopeClient::createGraphicsItem()
{
    int padding = 4;
    QGraphicsRectItem *item = new QGraphicsRectItem();
    ParameterGraphicsItem *parameterItem = new ParameterGraphicsItem(this);
    QRectF rect = QRect(0, 0, 1200, 420);
    rect = rect.translated(parameterItem->boundingRect().width() + 2 * padding, padding);
    parameterItem->setPos(padding, padding);
    parameterItem->setParentItem(item);
    EnvelopeGraphicsItem *ourItem = new EnvelopeGraphicsItem(rect, this);
    ourItem->setParentItem(item);
    item->setRect((rect | parameterItem->boundingRect().translated(parameterItem->pos())).adjusted(-padding, -padding, padding, padding));
    item->setPen(QPen(QBrush(Qt::black), 1));
    item->setBrush(QBrush(Qt::white));
    return item;
}

void EnvelopeClient::postChangeSustainIndex(int sustainIndex)
{
    Envelope::ChangeSustainPositionEvent *event = new Envelope::ChangeSustainPositionEvent(sustainIndex);
    guiEnvelope->processEvent(event, 0);
    postEvent(event);
}

double EnvelopeClient::evaluate(double x, int *index)
{
    return guiEnvelope->evaluate(x, index);
}

int EnvelopeClient::getNrOfControlPoints()
{
    return guiEnvelope->getNrOfControlPoints();
}

QPointF EnvelopeClient::getControlPoint(int index)
{
    return guiEnvelope->getControlPoint(index);
}

void EnvelopeClient::changeControlPoint(int index, double x, double y)
{
    guiEnvelope->changeControlPoint(index, x, y);
    // send the change to the process thread:
    Interpolator::ChangeControlPointEvent *event = new Interpolator::ChangeControlPointEvent(index, x, y);
    postEvent(event);
}

void EnvelopeClient::addControlPoint(double x, double y)
{
    guiEnvelope->addControlPoint(x, y);
    // this will have changed the bounds of the sustain index parameter:
    const ParameterProcessor::Parameter &parameter = guiEnvelope->getParameter(1);
    changedParameterValue(1, parameter.value, parameter.min, parameter.max);
    // send the change to the process thread:
    Interpolator::AddControlPointEvent *event = new Interpolator::AddControlPointEvent(x, y);
    postEvent(event);
}

void EnvelopeClient::deleteControlPoint(int index)
{
    guiEnvelope->deleteControlPoint(index);
    // this will have changed the bounds of the sustain index parameter:
    const ParameterProcessor::Parameter &parameter = guiEnvelope->getParameter(1);
    changedParameterValue(1, parameter.value, parameter.min, parameter.max);
    // send the change to the process thread:
    Interpolator::DeleteControlPointEvent *event = new Interpolator::DeleteControlPointEvent(index);
    postEvent(event);
}

QString EnvelopeClient::getControlPointName(int index) const
{
    return guiEnvelope->getControlPointName(index);
}

EnvelopeGraphicsItem::EnvelopeGraphicsItem(const QRectF &rect, EnvelopeClient *client_, QGraphicsItem *parent_) :
    GraphicsInterpolatorEditItem(
        client_,
        rect,
        QRectF(0, 1, client_->getControlPoint(client_->getNrOfControlPoints() - 1).x(), -2),
        parent_,
        4,
        8,
        true
        ),
    client(client_)
{
    setVisible(GraphicsInterpolatorEditItem::FIRST, false);
    setCursor(Qt::ArrowCursor);
    QObject::connect(client, SIGNAL(changedParameterValue(int,double,double,double)), this, SLOT(onChangedParameterValue(int)));
}

void EnvelopeGraphicsItem::onChangedParameterValue(int index)
{
    // update the interpolator if the sustain index has been changed:
    if (index == 1) {
        interpolatorChanged();
    }
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
