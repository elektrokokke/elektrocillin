#ifndef GRAPHICSCONTINOUSCONTROLITEM_H
#define GRAPHICSCONTINOUSCONTROLITEM_H

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

#include "graphicslabelitem.h"
#include "parameterprocessor.h"

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

    GraphicsContinuousControlItem(const QString &name, double minValue, double maxValue, double currentValue, qreal size, Orientation orientation = HORIZONTAL, char format = 'g', int precision = -1, double resolution = 0, QGraphicsItem *parent = 0);
    GraphicsContinuousControlItem(const ParameterProcessor::Parameter &parameter, qreal size, Orientation orientation = HORIZONTAL, char format = 'g', int precision = -1, QGraphicsItem *parent = 0);

    void setHorizontalAlignment(HorizontalAlignment horizontalAlignment);
public slots:
    void setMinValue(double minValue);
    void setMaxValue(double maxValue);
    void setSize(double size);
    void setValue(double value, bool emitSignal = true);
signals:
    void valueEditingStarted();
    void valueEditingStopped(double value);
    void valueChanged(double value);
protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant & currentValue);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
private:
    QString name;
    double minValue, maxValue, currentValue;
    QMap<double, QString> stringValues;
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
    QGraphicsRectItem *backgroundRect;
    GraphicsLabelItem *minLabel, *maxLabel;

private:
    qreal alignX(QRectF rect, QRectF rectAlign);
    void setLabelText(GraphicsLabelItem *label, double value);
};

#endif // GRAPHICSCONTINOUSCONTROLITEM_H
