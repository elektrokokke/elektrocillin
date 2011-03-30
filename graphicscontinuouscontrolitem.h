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
    enum HorizontalAlignment {
        LEFT,
        RIGHT
    };
    enum VerticalAlignment {
        TOP,
        BOTTOM
    };

    GraphicsContinuousControlItem(const QString &name, double minValue, double maxValue, double currentValue, qreal size, Orientation orientation = VERTICAL, char format = 'g', int precision = -1, double resolution = 0, QGraphicsItem *parent = 0);

    void setHorizontalAlignment(HorizontalAlignment horizontalAlignment);
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
    HorizontalAlignment horizontalAlignment;
    VerticalAlignment verticalAlignment;
    char format;
    int precision;
    double resolution;
    QPointF positionBeforeEdit;
    QRectF rectBeforeEdit;
    double valueBeforeEdit;
    bool waitingForMouseReleaseEvent;
    GraphicsLabelItem *minLabel, *maxLabel;

private:
    qreal alignX(QRectF rect, QRectF rectAlign);
};

#endif // GRAPHICSCONTINOUSCONTROLITEM_H
