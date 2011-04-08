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

PolynomialOscillatorClient::PolynomialOscillatorClient(const QString &clientName, PolynomialOscillator *processOscillator_, PolynomialOscillator *guiOscillator_, size_t ringBufferSize) :
    OscillatorClient(clientName, processOscillator_, guiOscillator_, processOscillator_, ringBufferSize),
    processOscillator(processOscillator_),
    guiOscillator(guiOscillator_)
{
}

PolynomialOscillatorClient::~PolynomialOscillatorClient()
{
    close();
}

void PolynomialOscillatorClient::saveState(QDataStream &stream)
{
    OscillatorClient::saveState(stream);
    guiOscillator->getPolynomialInterpolator()->save(stream);
}

void PolynomialOscillatorClient::loadState(QDataStream &stream)
{
    OscillatorClient::loadState(stream);
    guiOscillator->getPolynomialInterpolator()->load(stream);
    processOscillator->setPolynomialInterpolator(*guiOscillator->getPolynomialInterpolator());
}

QGraphicsItem * PolynomialOscillatorClient::createGraphicsItem()
{
    int padding = 4;
    QGraphicsRectItem *item = new QGraphicsRectItem();
    QGraphicsItem *oscillatorItem = OscillatorClient::createGraphicsItem();
    QRectF rect = QRect(0, 0, 420, 420);
    rect = rect.translated(oscillatorItem->boundingRect().width() + 2 * padding, padding);
    oscillatorItem->setPos(padding, padding);
    oscillatorItem->setParentItem(item);
    new GraphicsInterpolatorEditItem(this, rect, QRectF(0, 1, 1, -2), item);
    item->setRect((rect | oscillatorItem->boundingRect().translated(oscillatorItem->pos())).adjusted(-padding, -padding, padding, padding));
    item->setPen(QPen(QBrush(Qt::black), 1));
    item->setBrush(QBrush(Qt::white));
    return item;
}

double PolynomialOscillatorClient::evaluate(double x, int *index)
{
    return guiOscillator->getPolynomialInterpolator()->evaluate(x, index);
}

int PolynomialOscillatorClient::getNrOfControlPoints()
{
    return guiOscillator->getPolynomialInterpolator()->getNrOfControlPoints();
}

QPointF PolynomialOscillatorClient::getControlPoint(int index)
{
    return guiOscillator->getPolynomialInterpolator()->getControlPoint(index);
}

void PolynomialOscillatorClient::changeControlPoint(int index, double x, double y)
{
    guiOscillator->getPolynomialInterpolator()->changeControlPoint(index, x, y);
    Interpolator::ChangeControlPointEvent *event = new Interpolator::ChangeControlPointEvent(index, x, y);
    postEvent(event);
}

void PolynomialOscillatorClient::addControlPoint(double x, double y)
{
    guiOscillator->getPolynomialInterpolator()->addControlPoint(x, y);
    Interpolator::AddControlPointEvent *event = new Interpolator::AddControlPointEvent(x, y);
    postEvent(event);
}

void PolynomialOscillatorClient::deleteControlPoint(int index)
{
    guiOscillator->getPolynomialInterpolator()->deleteControlPoint(index);
    Interpolator::DeleteControlPointEvent *event = new Interpolator::DeleteControlPointEvent(index);
    postEvent(event);
}

QString PolynomialOscillatorClient::getControlPointName(int index) const
{
    return guiOscillator->getPolynomialInterpolator()->getControlPointName(index);
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
        return "Oscillator (linear)";
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
