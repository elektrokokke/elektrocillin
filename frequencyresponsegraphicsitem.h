#ifndef FREQUENCYRESPONSEGRAPHICSITEM_H
#define FREQUENCYRESPONSEGRAPHICSITEM_H

#include <QGraphicsRectItem>
#include <QPen>

class QGraphicsPathItem;
class FrequencyResponse;

class FrequencyResponseGraphicsItem : public QGraphicsRectItem
{
public:
    FrequencyResponseGraphicsItem(const QRectF &fitInto, double lowestHertz, double highestHertz, double lowestDecibel = -40, double highestDecibel = 40, QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    FrequencyResponseGraphicsItem(qreal x, qreal y, qreal w, qreal h, double lowestHertz, double highestHertz, double lowestDecibel = -40, double highestDecibel = 40, QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);

    int addFrequencyResponse(FrequencyResponse *response);
    void updateFrequencyResponse(int index);
    void updateFrequencyResponses();

    double getLowestHertz() const;
    double getHighestHertz() const;

    const QRectF & getFrequencyResponseRectangle() const;
    qreal getZeroDecibelY() const;

private:
    double lowestHertz, highestHertz, lowestDecibel, highestDecibel;
    QRectF innerRectangle;
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
