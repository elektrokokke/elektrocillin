#include "frequencyresponsegraphicsitem.h"
#include <QGraphicsSimpleTextItem>
#include <QPen>
#include <QtGlobal>
#include <cmath>

FrequencyResponseGraphicsItem::FrequencyResponseGraphicsItem(const QRectF &rect, double lowestHertz_, double highestHertz_, double lowestDecibel_, double highestDecibel_, QGraphicsItem *parent, QGraphicsScene *scene) :
    QGraphicsRectItem(rect, parent, scene),
    lowestHertz(lowestHertz_),
    highestHertz(highestHertz_),
    lowestDecibel(lowestDecibel_),
    highestDecibel(highestDecibel_),
    frequencyResponseColor(Qt::red)
{
    initialize();
}

FrequencyResponseGraphicsItem::FrequencyResponseGraphicsItem(qreal x, qreal y, qreal w, qreal h, double lowestHertz_, double highestHertz_, double lowestDecibel_, double highestDecibel_, QGraphicsItem *parent, QGraphicsScene *scene) :
    QGraphicsRectItem(x, y, w, h, parent, scene),
    lowestHertz(lowestHertz_),
    highestHertz(highestHertz_),
    lowestDecibel(lowestDecibel_),
    highestDecibel(highestDecibel_),
    frequencyResponseColor(Qt::red)
{
    initialize();
}

void FrequencyResponseGraphicsItem::updateFrequencyResponse()
{
    qreal innerTop = innerRectangle.top();
    qreal innerBottom = innerRectangle.bottom();
    for (int i = 0; i < frequencies.size(); i++) {
        double squaredAmplitudeResponse = getSquaredAmplitudeResponse(frequencies[i]);
        // follow the 10 * log10 rule:
        double decibel = (squaredAmplitudeResponse == 0 ? lowestDecibel : qMax(lowestDecibel, qMin(highestDecibel, 10.0 * log(squaredAmplitudeResponse) / log(10.0))));
        qreal y = (decibel - lowestDecibel) / (highestDecibel - lowestDecibel) * (innerTop - innerBottom) + innerBottom;
        if (i > 0) {
            QLineF line = frequencyResponseLines[i - 1]->line();
            frequencyResponseLines[i - 1]->setLine(line.p1().x(), line.p1().y(), line.p2().x(), y);
        }
        if (i < frequencyResponseLines.size()) {
            QLineF line = frequencyResponseLines[i]->line();
            frequencyResponseLines[i]->setLine(line.p1().x(), y, line.p2().x(), line.p2().y());
        }
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

double FrequencyResponseGraphicsItem::getSquaredAmplitudeResponse(double frequencyInHertz)
{
    return 1.0;
}

void FrequencyResponseGraphicsItem::initialize()
{
    qreal tickSize = 10;
    double frequencyTickFactor = 2.0;
    double frequencyResponseFactor = pow(2.0, 1.0 / 24.0);

    // create the first horizontal textual label:
    QGraphicsSimpleTextItem *horizontalLabel = new QGraphicsSimpleTextItem(QString("%1Hz").arg(lowestHertz, 0, 'g', 5), this);
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
        if (verticalLabel->boundingRect().width() > maxLabelWidth) {
            maxLabelWidth = verticalLabel->boundingRect().width();
        }
        // move it to the right position:
        verticalLabel->setPos(padding, y - padding * 0.5);
    }
    for (double decibel =-10.0; decibel >= lowestDecibel; decibel -= 10.0) {
        // compute the vertical position of the label:
        qreal y = (decibel - lowestDecibel) / (highestDecibel - lowestDecibel) * (innerTop - innerBottom) + innerBottom;
        // create a vertical label:
        QGraphicsSimpleTextItem *verticalLabel = new QGraphicsSimpleTextItem(QString("%1dB").arg(decibel), this);
        if (verticalLabel->boundingRect().width() > maxLabelWidth) {
            maxLabelWidth = verticalLabel->boundingRect().width();
        }
        // move it to the right position:
        verticalLabel->setPos(padding, y - padding * 0.5);
    }
    // compute the inner rectangle's left and right:
    qreal innerLeft = rect().left() + maxLabelWidth + padding + tickSize;
    qreal innerRight = rect().right() - padding;
    innerRectangle = QRectF(innerLeft, innerTop, innerRight - innerLeft, innerBottom - innerTop);
    // create the vertical ticks and dotted vertical lines:
    for (double decibel = 0.0; decibel <= highestDecibel; decibel += 10.0) {
        qreal y = (decibel - lowestDecibel) / (highestDecibel - lowestDecibel) * (innerTop - innerBottom) + innerBottom;
        new QGraphicsLineItem(innerLeft - tickSize * 0.5, y, innerLeft, y, this);
        (new QGraphicsLineItem(innerLeft, y, innerRight, y, this))->setPen(QPen(Qt::DotLine));

    }
    for (double decibel =-10.0; decibel >= lowestDecibel; decibel -= 10.0) {
        qreal y = (decibel - lowestDecibel) / (highestDecibel - lowestDecibel) * (innerTop - innerBottom) + innerBottom;
        new QGraphicsLineItem(innerLeft - tickSize * 0.5, y, innerLeft, y, this);
        (new QGraphicsLineItem(innerLeft, y, innerRight, y, this))->setPen(QPen(Qt::DotLine));
    }
    // move the first horizontal label to the right position:
    horizontalLabel->setPos(innerLeft - horizontalLabel->boundingRect().width() * 0.5, innerBottom + tickSize);
    qreal lastRight = innerLeft + horizontalLabel->boundingRect().width() * 0.5;
    // create horizontal ticks and dotted horizontal lines:
    for (double hertz = lowestHertz; hertz <= highestHertz; hertz *= frequencyTickFactor) {
        qreal x = log(hertz / lowestHertz) / log(highestHertz / lowestHertz) * (innerRight - innerLeft) + innerLeft;
        new QGraphicsLineItem(x, innerBottom, x, innerBottom + tickSize * 0.5, this);
        (new QGraphicsLineItem(x, innerTop, x, innerBottom, this))->setPen(QPen(Qt::DotLine));
        // create another horizontal label if it fits:
        QGraphicsSimpleTextItem *label = new QGraphicsSimpleTextItem(QString("%1Hz").arg(hertz, 0, 'g', 5));
        if ((x - label->boundingRect().width() * 0.5 > lastRight) && (x + label->boundingRect().width() * 0.5 < boundingRect().right())) {
            label->setParentItem(this);
            label->setPos(x - label->boundingRect().width() * 0.5, innerBottom + tickSize);
            lastRight = x + label->boundingRect().width() * 0.5;
        } else {
            delete label;
        }
    }
    // create the lines showing the frequency response:
    for (double hertz = lowestHertz; hertz < highestHertz; hertz *= frequencyResponseFactor) {
        frequencies.append(hertz);
    }
    frequencies.append(highestHertz);
    qreal xPrevious = innerLeft;
    for (int i = 1; i < frequencies.size(); i++) {
        qreal x = log(frequencies[i] / lowestHertz) / log(highestHertz / lowestHertz) * (innerRight - innerLeft) + innerLeft;
        qreal y = - lowestDecibel / (highestDecibel - lowestDecibel) * (innerTop - innerBottom) + innerBottom;
        QGraphicsLineItem *frequencyResponseLine = new QGraphicsLineItem(xPrevious, y, x, y, this);
        frequencyResponseLine->setPen(QPen(QBrush(frequencyResponseColor), 2));
        frequencyResponseLine->setZValue(1);
        frequencyResponseLines.append(frequencyResponseLine);
        xPrevious = x;
    }
}
