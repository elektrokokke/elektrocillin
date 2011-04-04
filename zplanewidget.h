#ifndef ZPLANEWIDGET_H
#define ZPLANEWIDGET_H

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

#include <QWidget>
#include <QVector>
#include <QGraphicsLineItem>
#include "zplanefilter.h"

namespace Ui {
    class ZPlaneWidget;
}

class QGraphicsEllipseItem;
class QGraphicsRectItem;
class QGraphicsScene;

class ZPlaneWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ZPlaneWidget(QWidget *parent = 0);
    ~ZPlaneWidget();

private:
    Ui::ZPlaneWidget *ui;
    int xscale, yscale;
    ZPlaneFilter filter2pole;
    QVector<double> squaredAmplitudeResponse2pole;
    QGraphicsScene *scene;
    QVector<QGraphicsLineItem*> lines2pole;
    QVector<QGraphicsEllipseItem*> zeroItems;
    QVector<QGraphicsRectItem*> poleItems;

    void drawButterworthPoleTrail();

private slots:
    void updateFrequencyResponse2pole();
};

#endif // ZPLANEWIDGET_H
