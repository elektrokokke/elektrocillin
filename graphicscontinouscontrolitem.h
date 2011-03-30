#ifndef GRAPHICSCONTINOUSCONTROLITEM_H
#define GRAPHICSCONTINOUSCONTROLITEM_H

#include "graphicslabelitem.h"

class GraphicsContinousControlItem : public GraphicsLabelItem
{
    Q_OBJECT
public:
    enum Orientation {
        HORIZONTAL,
        VERTICAL
    };

    GraphicsContinousControlItem(const QString &name, double minValue, double maxValue, double currentValue, qreal size, Orientation orientation = VERTICAL, int precision = -1, double resolution = 0, QGraphicsItem *parent = 0);
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
    int precision;
    double resolution;
    QPointF positionBeforeEdit;
    double valueBeforeEdit;
    bool waitingForMouseReleaseEvent;
};

#endif // GRAPHICSCONTINOUSCONTROLITEM_H
