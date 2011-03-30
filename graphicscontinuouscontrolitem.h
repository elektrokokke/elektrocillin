#ifndef GRAPHICSCONTINOUSCONTROLITEM_H
#define GRAPHICSCONTINOUSCONTROLITEM_H

#include "graphicslabelitem.h"

class GraphicsContinuousControlItem : public GraphicsLabelItem
{
    Q_OBJECT
public:
    enum Orientation {
        HORIZONTAL,
        VERTICAL
    };

    GraphicsContinuousControlItem(const QString &name, double minValue, double maxValue, double currentValue, qreal size, Orientation orientation = VERTICAL, char format = 'g', int precision = -1, double resolution = 0, QGraphicsItem *parent = 0);
public slots:
signals:
    void valueEditingStarted();
    void valueEditingStopped();
    void valueChanged(double value);
protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant & currentValue);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
private:
    QString name;
    double minValue, maxValue, currentValue;
    double size;
    Orientation orientation;
    char format;
    int precision;
    double resolution;
    QPointF positionBeforeEdit;
    double valueBeforeEdit;
    bool waitingForMouseReleaseEvent;
    GraphicsLabelItem *minLabel, *maxLabel;
};

#endif // GRAPHICSCONTINOUSCONTROLITEM_H
