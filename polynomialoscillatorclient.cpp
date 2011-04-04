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

#include "polynomialoscillatorclient.h"

PolynomialOscillatorClient::PolynomialOscillatorClient(const QString &clientName, PolynomialOscillator *processOscillator, PolynomialOscillator *guiOscillator, size_t ringBufferSize) :
    OscillatorClient(clientName, processOscillator, guiOscillator, processOscillator, ringBufferSize),
    oscillator(3)
{
}

PolynomialOscillatorClient::~PolynomialOscillatorClient()
{
    close();
}

void PolynomialOscillatorClient::saveState(QDataStream &stream)
{
    OscillatorClient::saveState(stream);
    oscillator.getPolynomialInterpolator()->save(stream);
}

void PolynomialOscillatorClient::loadState(QDataStream &stream)
{
    OscillatorClient::loadState(stream);
    PolynomialInterpolator interpolator;
    interpolator.load(stream);
    oscillator.setPolynomialInterpolator(interpolator);
    getIntegralOscillator()->setPolynomialInterpolator(interpolator);
}

PolynomialInterpolator * PolynomialOscillatorClient::getPolynomialInterpolator()
{
    return oscillator.getPolynomialInterpolator();
}

void PolynomialOscillatorClient::postIncreaseControlPoints()
{
    InterpolatorProcessor::AddControlPointsEvent *event = new InterpolatorProcessor::AddControlPointsEvent(true, false, false, true);
    oscillator.processEvent(event, 0);
    postEvent(event);
}

void PolynomialOscillatorClient::postDecreaseControlPoints()
{
    if (oscillator.getPolynomialInterpolator()->getX().size() > 2) {
        InterpolatorProcessor::DeleteControlPointsEvent *event = new InterpolatorProcessor::DeleteControlPointsEvent(true, false, false, true);
        oscillator.processEvent(event, 0);
        postEvent(event);
    }
}

void PolynomialOscillatorClient::postChangeControlPoint(int index, double x, double y)
{
    InterpolatorProcessor::ChangeControlPointEvent *event = new InterpolatorProcessor::ChangeControlPointEvent(index, x, y);
    oscillator.processEvent(event, 0);
    postEvent(event);
}

PolynomialOscillator * PolynomialOscillatorClient::getIntegralOscillator()
{
    return (PolynomialOscillator*)getAudioProcessor();
}

QGraphicsItem * PolynomialOscillatorClient::createGraphicsItem()
{
    int padding = 4;
    QGraphicsRectItem *item = new QGraphicsRectItem();
    QGraphicsItem *oscillatorItem = OscillatorClient::createGraphicsItem();
    QRectF rect = QRect(0, 0, 600, 420);
    rect = rect.translated(oscillatorItem->boundingRect().width() + 2 * padding, padding);
    oscillatorItem->setPos(padding, padding);
    oscillatorItem->setParentItem(item);
    QGraphicsItem *ourItem = new IntegralOscillatorGraphicsItem(rect, this);
    ourItem->setParentItem(item);
    item->setRect((rect | oscillatorItem->boundingRect().translated(oscillatorItem->pos())).adjusted(-padding, -padding, padding, padding));
    item->setPen(QPen(QBrush(Qt::black), 1));
    item->setBrush(QBrush(Qt::white));
    return item;
}

IntegralOscillatorGraphicsItem::IntegralOscillatorGraphicsItem(const QRectF &rect, PolynomialOscillatorClient *client_, QGraphicsItem *parent) :
    GraphicsInterpolatorEditItem(
        client_->getPolynomialInterpolator(),
        rect,
        QRectF(client_->getPolynomialInterpolator()->getX().first(), 1, client_->getPolynomialInterpolator()->getX().last() - client_->getPolynomialInterpolator()->getX().first(), -2),
        parent
        ),
    client(client_)
{}

void IntegralOscillatorGraphicsItem::increaseControlPoints()
{
    client->postIncreaseControlPoints();
}

void IntegralOscillatorGraphicsItem::decreaseControlPoints()
{
    client->postDecreaseControlPoints();
}

void IntegralOscillatorGraphicsItem::changeControlPoint(int index, double x, double y)
{
    client->postChangeControlPoint(index, x, y);
}

class IntegralOscillatorClientFactory : public JackClientFactory
{
public:
    IntegralOscillatorClientFactory()
    {
        JackClientSerializer::getInstance()->registerFactory(this);
    }
    QString getName()
    {
        return "Polynomial oscillator";
    }
    JackClient * createClient(const QString &clientName)
    {
        return new PolynomialOscillatorClient(clientName, new PolynomialOscillator(3), new PolynomialOscillator(3));
    }
    static IntegralOscillatorClientFactory factory;
};

IntegralOscillatorClientFactory IntegralOscillatorClientFactory::factory;

JackClientFactory * PolynomialOscillatorClient::getFactory()
{
    return &IntegralOscillatorClientFactory::factory;
}
