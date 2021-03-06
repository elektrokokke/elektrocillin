/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Elektrocillin is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Elektrocillin.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "iirmoogfilterclient.h"

IirMoogFilterClient::IirMoogFilterClient(const QString &clientName, IirMoogFilter *processFilter_, IirMoogFilter *guiFilter_, size_t ringBufferSize) :
    ParameterClient(clientName, processFilter_, processFilter_, 0, processFilter_, guiFilter_, ringBufferSize),
    processFilter(processFilter_),
    guiFilter(guiFilter_)
{
}

IirMoogFilterClient::~IirMoogFilterClient()
{
    // calling close will stop the Jack client:
    close();
    // deleting the filter is now safe, as it is not used anymore (the Jack process thread is stopped):
    delete processFilter;
    delete guiFilter;
}

IirMoogFilter * IirMoogFilterClient::getMoogFilter()
{
    return guiFilter;
}

QGraphicsItem * IirMoogFilterClient::createGraphicsItem()
{
    int padding = 4;
    QGraphicsRectItem *item = new QGraphicsRectItem();
    QGraphicsItem *parameterItem = ParameterClient::createGraphicsItem();
    QRectF rect = QRect(0, 0, 600, 420);
    rect = rect.translated(parameterItem->boundingRect().width() + 2 * padding, padding);
    parameterItem->setPos(padding, padding);
    parameterItem->setParentItem(item);
    QGraphicsItem *ourItem = new IirMoogFilterGraphicsItem(this, rect);
    ourItem->setParentItem(item);
    item->setRect((rect | parameterItem->boundingRect().translated(parameterItem->pos())).adjusted(-padding, -padding, padding, padding));
    item->setPen(QPen(QBrush(Qt::black), 1));
    item->setBrush(QBrush(Qt::white));
    return item;
}

bool IirMoogFilterClient::init()
{
    if (ParameterClient::init()) {
        // adjust the guiFilter's samplerate to that of the processFilter:
        guiFilter->setSampleRate(processFilter->getSampleRate());
        return true;
    } else {
        return false;
    }
}

IirMoogFilterGraphicsItem::IirMoogFilterGraphicsItem(IirMoogFilterClient *client_, const QRectF &rect, QGraphicsItem *parent) :
    FrequencyResponseGraphicsItem(rect, 22050.0 / 512.0, 22050, -30, 30, parent),
    client(client_)
{
    addFrequencyResponse(client->getMoogFilter());
    cutoffResonanceNode = new GraphicsNodeItem(-5.0, -5.0, 10.0, 10.0, this);
    cutoffResonanceNode->setScale(GraphicsNodeItem::LOGARITHMIC, GraphicsNodeItem::LINEAR);
    cutoffResonanceNode->setPen(QPen(QBrush(qRgb(114, 159, 207)), 3));
    cutoffResonanceNode->setBrush(QBrush(qRgb(52, 101, 164)));
    cutoffResonanceNode->setZValue(10);
    cutoffResonanceNode->setBounds(QRectF(getFrequencyResponseRectangle().topLeft(), QPointF(getFrequencyResponseRectangle().right(), getZeroDecibelY())));
    cutoffResonanceNode->setBoundsScaled(QRectF(QPointF(getLowestHertz(), 1), QPointF(getHighestHertz(), 0)));
    onClientChangedParameters();
    QObject::connect(cutoffResonanceNode, SIGNAL(positionChangedScaled(QPointF)), this, SLOT(onGuiChangedFilterParameters(QPointF)));
    QObject::connect(client, SIGNAL(changedParameterValue(int,double,double,double)), this, SLOT(onClientChangedParameters()));
}

void IirMoogFilterGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (client->getMoogFilter()->computeCoefficients()) {
        updateFrequencyResponse(0);
    }
    FrequencyResponseGraphicsItem::paint(painter, option, widget);
}

void IirMoogFilterGraphicsItem::onClientChangedParameters()
{
    if (!cutoffResonanceNode->isMoving()) {
        cutoffResonanceNode->setXScaled(client->getMoogFilter()->getBaseCutoffFrequency());
        cutoffResonanceNode->setYScaled(client->getMoogFilter()->getResonance());
    }
    update();
}

void IirMoogFilterGraphicsItem::onGuiChangedFilterParameters(const QPointF &cutoffResonance)
{
    client->changeParameterValue(1, cutoffResonance.x());
    client->changeParameterValue(2, cutoffResonance.y());
}

class IirMoogFilterClientFactory : public JackClientFactory
{
public:
    IirMoogFilterClientFactory()
    {
        JackClientSerializer::getInstance()->registerFactory(this);
    }
    QString getName()
    {
        return "Filter (low-pass)";
    }
    JackClient * createClient(const QString &clientName)
    {
        return new IirMoogFilterClient(clientName, new IirMoogFilter(1), new IirMoogFilter(1));
    }
    static IirMoogFilterClientFactory factory;
};

IirMoogFilterClientFactory IirMoogFilterClientFactory::factory;

JackClientFactory * IirMoogFilterClient::getFactory()
{
    return &IirMoogFilterClientFactory::factory;
}
