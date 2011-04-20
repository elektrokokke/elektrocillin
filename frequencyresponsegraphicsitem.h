#ifndef FREQUENCYRESPONSEGRAPHICSITEM_H
#define FREQUENCYRESPONSEGRAPHICSITEM_H

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

#include <QGraphicsRectItem>
#include <QPen>
#include <QFont>

class QGraphicsPathItem;
class FrequencyResponse;

class FrequencyResponseGraphicsItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    FrequencyResponseGraphicsItem(const QRectF &fitInto, double lowestHertz, double highestHertz, double lowestDecibel = -40, double highestDecibel = 40, QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    FrequencyResponseGraphicsItem(qreal x, qreal y, qreal w, qreal h, double lowestHertz, double highestHertz, double lowestDecibel = -40, double highestDecibel = 40, QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);

    int addFrequencyResponse(FrequencyResponse *response);
    void updateFrequencyResponse(int index);

    double getLowestHertz() const;
    double getHighestHertz() const;

    const QRectF & getFrequencyResponseRectangle() const;
    qreal getZeroDecibelY() const;

public slots:
    void updateFrequencyResponses();
private:
    double lowestHertz, highestHertz, lowestDecibel, highestDecibel;
    QRectF innerRectangle;
    QFont font;
    qreal tickSize;
    double frequencyTickFactor;
    double frequencyResponseFactor;
    QVector<QPen> frequencyResponsePens;
    QVector<double> frequencies;
    QVector<qreal> xPositions;
    QVector<FrequencyResponse*> frequencyResponses;
    QVector<QGraphicsPathItem*> frequencyResponseLines;

    void initialize();
};

#endif // FREQUENCYRESPONSEGRAPHICSITEM_H
