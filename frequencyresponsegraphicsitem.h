#ifndef FREQUENCYRESPONSEGRAPHICSITEM_H
#define FREQUENCYRESPONSEGRAPHICSITEM_H

#include <QGraphicsRectItem>
#include <QColor>

class QGraphicsLineItem;

class FrequencyResponseGraphicsItem : public QGraphicsRectItem
{
public:
    FrequencyResponseGraphicsItem(const QRectF &rect, double lowestHertz, double highestHertz, double lowestDecibel = -40, double highestDecibel = 40, QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    FrequencyResponseGraphicsItem(qreal x, qreal y, qreal w, qreal h, double lowestHertz, double highestHertz, double lowestDecibel = -40, double highestDecibel = 40, QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);

    void updateFrequencyResponse();

    double getLowestHertz() const;
    double getHighestHertz() const;

protected:
    virtual double getSquaredAmplitudeResponse(double frequencyInHertz);

private:
    double lowestHertz, highestHertz, lowestDecibel, highestDecibel;
    QRectF innerRectangle;
    qreal tickSize;
    double frequencyTickFactor;
    double frequencyResponseFactor;
    QColor frequencyResponseColor;
    QVector<double> frequencies;
    QVector<QGraphicsLineItem*> frequencyResponseLines;

    void initialize();
};

#endif // FREQUENCYRESPONSEGRAPHICSITEM_H
