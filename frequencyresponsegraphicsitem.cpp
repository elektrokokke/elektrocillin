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

#include "frequencyresponsegraphicsitem.h"
#include "frequencyresponse.h"
#include <QGraphicsSimpleTextItem>
#include <QPen>
#include <QtGlobal>
#include <cmath>

FrequencyResponseGraphicsItem::FrequencyResponseGraphicsItem(const QRectF &rect, double lowestHertz_, double highestHertz_, double lowestDecibel_, double highestDecibel_, QGraphicsItem *parent, QGraphicsScene *scene) :
    QGraphicsRectItem(rect, parent, scene),
    lowestHertz(lowestHertz_),
    highestHertz(highestHertz_),
    lowestDecibel(lowestDecibel_),
    highestDecibel(highestDecibel_)
{
    initialize();
}

FrequencyResponseGraphicsItem::FrequencyResponseGraphicsItem(qreal x, qreal y, qreal w, qreal h, double lowestHertz_, double highestHertz_, double lowestDecibel_, double highestDecibel_, QGraphicsItem *parent, QGraphicsScene *scene) :
    QGraphicsRectItem(x, y, w, h, parent, scene),
    lowestHertz(lowestHertz_),
    highestHertz(highestHertz_),
    lowestDecibel(lowestDecibel_),
    highestDecibel(highestDecibel_)
{
    initialize();
}

int FrequencyResponseGraphicsItem::addFrequencyResponse(FrequencyResponse *response)
{
    int index = frequencyResponses.size();
    frequencyResponses.append(response);
    frequencyResponseLines.append(new QGraphicsPathItem(this));
    frequencyResponseLines[index]->setPen(frequencyResponsePens[index % frequencyResponsePens.size()]);
    updateFrequencyResponse(index);
    return index;
}

void FrequencyResponseGraphicsItem::updateFrequencyResponse(int responseIndex)
{
    QPainterPath path;
    qreal innerTop = innerRectangle.top();
    qreal innerBottom = innerRectangle.bottom();
    for (int i = 0; i < frequencies.size(); i++) {
        double squaredAmplitudeResponse = frequencyResponses[responseIndex]->getSquaredAmplitudeResponse(frequencies[i]);
        // follow the 10 * log10 rule:
        double decibel = (squaredAmplitudeResponse == 0 ? lowestDecibel : qMax(lowestDecibel, qMin(highestDecibel, 10.0 * log(squaredAmplitudeResponse) / log(10.0))));
        qreal x = xPositions[i];
        qreal y = (decibel - lowestDecibel) / (highestDecibel - lowestDecibel) * (innerTop - innerBottom) + innerBottom;
        if (i == 0) {
            path.moveTo(x, y);
        } else {
            path.lineTo(x, y);
        }
    }
    frequencyResponseLines[responseIndex]->setPath(path);
}

void FrequencyResponseGraphicsItem::updateFrequencyResponses()
{
    for (int responseIndex = 0; responseIndex < frequencyResponses.size(); responseIndex++) {
        updateFrequencyResponse(responseIndex);
    }
}

double FrequencyResponseGraphicsItem::getLowestHertz() const
{
    return lowestHertz;
}

double FrequencyResponseGraphicsItem::getHighestHertz() const
{
    return highestHertz;
}

const QRectF & FrequencyResponseGraphicsItem::getFrequencyResponseRectangle() const
{
    return innerRectangle;
}

qreal FrequencyResponseGraphicsItem::getZeroDecibelY() const
{
    qreal innerTop = innerRectangle.top();
    qreal innerBottom = innerRectangle.bottom();
    return - lowestDecibel / (highestDecibel - lowestDecibel) * (innerTop - innerBottom) + innerBottom;
}

void FrequencyResponseGraphicsItem::initialize()
{
    setPen(QPen(QBrush(Qt::black), 1));
    setBrush(QBrush(Qt::white));
    font.setPointSize(6);

    frequencyResponsePens.append(QPen(QBrush(Qt::black), 2));
    frequencyResponsePens.append(QPen(QBrush(Qt::darkRed), 2));
    frequencyResponsePens.append(QPen(QBrush(Qt::darkGreen), 2));
    frequencyResponsePens.append(QPen(QBrush(Qt::darkBlue), 2));
    frequencyResponsePens.append(QPen(QBrush(Qt::gray), 2, Qt::DashLine));
    frequencyResponsePens.append(QPen(QBrush(Qt::darkRed), 2, Qt::DashLine));
    frequencyResponsePens.append(QPen(QBrush(Qt::darkGreen), 2, Qt::DashLine));
    frequencyResponsePens.append(QPen(QBrush(Qt::darkBlue), 2, Qt::DashLine));

    qreal tickSize = 10;
    double frequencyTickFactor = 2.0;
    double frequencyResponseFactor = pow(2.0, 1.0 / 24.0);

    // create the first horizontal textual label:
    QGraphicsSimpleTextItem *horizontalLabel = new QGraphicsSimpleTextItem(QString("%1Hz").arg(lowestHertz, 0, 'g', 5), this);
    horizontalLabel->setFont(font);
    qreal padding = horizontalLabel->boundingRect().height();
    // compute the inner rectangle's bottom and top:
    qreal innerBottom = rect().bottom() - tickSize - padding * 2;
    qreal innerTop = rect().top() + padding;
    // now create the vertical labels, put them at zero and multiples of 10:
    qreal maxLabelWidth = horizontalLabel->boundingRect().width() * 0.5 - tickSize;
    for (double decibel = 0.0; decibel <= highestDecibel; decibel += 10.0) {
        // compute the vertical position of the label:
        // (decibel == lowestDecibel => y == innerBottom)
        // (decibel == highestDecibel => y == innerTop)
        qreal y = (decibel - lowestDecibel) / (highestDecibel - lowestDecibel) * (innerTop - innerBottom) + innerBottom;
        // create a vertical label:
        QGraphicsSimpleTextItem *verticalLabel = new QGraphicsSimpleTextItem(QString("%1dB").arg(decibel), this);
        verticalLabel->setFont(font);
        if (verticalLabel->boundingRect().width() > maxLabelWidth) {
            maxLabelWidth = verticalLabel->boundingRect().width();
        }
        // move it to the right position:
        verticalLabel->setPos(rect().left() + padding, y - padding * 0.5);
    }
    for (double decibel =-10.0; decibel >= lowestDecibel; decibel -= 10.0) {
        // compute the vertical position of the label:
        qreal y = (decibel - lowestDecibel) / (highestDecibel - lowestDecibel) * (innerTop - innerBottom) + innerBottom;
        // create a vertical label:
        QGraphicsSimpleTextItem *verticalLabel = new QGraphicsSimpleTextItem(QString("%1dB").arg(decibel), this);
        verticalLabel->setFont(font);
        if (verticalLabel->boundingRect().width() > maxLabelWidth) {
            maxLabelWidth = verticalLabel->boundingRect().width();
        }
        // move it to the right position:
        verticalLabel->setPos(rect().left() + padding, y - padding * 0.5);
    }
    // compute the inner rectangle's left and right:
    qreal innerLeft = rect().left() + maxLabelWidth + padding + tickSize;
    qreal innerRight = rect().right() - padding;
    innerRectangle = QRectF(innerLeft, innerTop, innerRight - innerLeft, innerBottom - innerTop);
    // create the vertical ticks and dotted vertical lines:
    for (double decibel = 0.0; decibel <= highestDecibel; decibel += 10.0) {
        qreal y = (decibel - lowestDecibel) / (highestDecibel - lowestDecibel) * (innerTop - innerBottom) + innerBottom;
        (new QGraphicsLineItem(innerLeft - tickSize * 0.5, y, innerLeft, y, this))->setPen(QPen(QBrush(Qt::black), 0));
        (new QGraphicsLineItem(innerLeft, y, innerRight, y, this))->setPen(QPen(QBrush(Qt::black), 0, Qt::DotLine));
    }
    for (double decibel =-10.0; decibel >= lowestDecibel; decibel -= 10.0) {
        qreal y = (decibel - lowestDecibel) / (highestDecibel - lowestDecibel) * (innerTop - innerBottom) + innerBottom;
        (new QGraphicsLineItem(innerLeft - tickSize * 0.5, y, innerLeft, y, this))->setPen(QPen(QBrush(Qt::black), 0));
        (new QGraphicsLineItem(innerLeft, y, innerRight, y, this))->setPen(QPen(QBrush(Qt::black), 0, Qt::DotLine));
    }
    // move the first horizontal label to the right position:
    horizontalLabel->setPos(innerLeft - horizontalLabel->boundingRect().width() * 0.5, innerBottom + tickSize);
    qreal lastRight = innerLeft + horizontalLabel->boundingRect().width() * 0.5;
    // create horizontal ticks and dotted horizontal lines:
    for (double hertz = lowestHertz; hertz <= highestHertz; hertz *= frequencyTickFactor) {
        qreal x = log(hertz / lowestHertz) / log(highestHertz / lowestHertz) * (innerRight - innerLeft) + innerLeft;
        (new QGraphicsLineItem(x, innerBottom, x, innerBottom + tickSize * 0.5, this))->setPen(QPen(QBrush(Qt::black), 0));
        (new QGraphicsLineItem(x, innerTop, x, innerBottom, this))->setPen(QPen(QBrush(Qt::black), 0, Qt::DotLine));
        // create another horizontal label if it fits:
        QGraphicsSimpleTextItem *label = new QGraphicsSimpleTextItem(QString("%1Hz").arg(hertz, 0, 'g', 5));
        label->setFont(font);
        if ((x - label->boundingRect().width() * 0.5 > lastRight) && (x + label->boundingRect().width() * 0.5 < rect().right())) {
            label->setParentItem(this);
            label->setPos(x - label->boundingRect().width() * 0.5, innerBottom + tickSize);
            lastRight = x + label->boundingRect().width() * 0.5;
        } else {
            delete label;
        }
    }
    // compute the frequencies at which the responses are evaluated:
    for (double hertz = lowestHertz; hertz < highestHertz; hertz *= frequencyResponseFactor) {
        frequencies.append(hertz);
        qreal x = log(hertz / lowestHertz) / log(highestHertz / lowestHertz) * (innerRight - innerLeft) + innerLeft;
        xPositions.append(x);
    }
    frequencies.append(highestHertz);
    xPositions.append(innerRight);
}
