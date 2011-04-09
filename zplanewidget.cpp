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

#include "zplanewidget.h"
#include "ui_zplanewidget.h"
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QDebug>
#include <cmath>

ZPlaneWidget::ZPlaneWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ZPlaneWidget),
    xscale(5),
    yscale(10)
{
    ui->setupUi(this);
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);

    int max = 80;
    squaredAmplitudeResponse2pole.resize(257);
    filter2pole.addPole(0.0);
    filter2pole.addPole(0.0);
    filter2pole.addZero(0.0);
    filter2pole.addZero(0.0);

    for (int i = 0; i < squaredAmplitudeResponse2pole.size() - 1; i++) {
        lines2pole.append(scene->addLine(i * xscale, 0, (i+1) * xscale, 0, QPen(Qt::red)));
    }
    scene->addLine(0, 0, squaredAmplitudeResponse2pole.size() * xscale, 0);
    scene->addLine(0, max * yscale, 0, -max * yscale);
    for (int i = -max; i <= max; i++) {
        scene->addText(QString::number(i))->setPos(-20, -i * yscale);
        scene->addLine(-5, -i * yscale, 0, -i * yscale);
    }

    // be notified when the user changes the filter parameters:
    QObject::connect(ui->spinBoxFrequency, SIGNAL(valueChanged(double)), this, SLOT(updateFrequencyResponse2pole()));

    scene->addEllipse(350-250, 350-250, 500, 500);
    zeroItems.append(scene->addEllipse(-5, -5, 10, 10));
    zeroItems.append(scene->addEllipse(-5, -5, 10, 10));
    poleItems.append(scene->addRect(-5, -5, 10, 10));
    poleItems.append(scene->addRect(-5, -5, 10, 10));

    updateFrequencyResponse2pole();

    drawButterworthPoleTrail();
}

ZPlaneWidget::~ZPlaneWidget()
{
    delete ui;
}

void ZPlaneWidget::updateFrequencyResponse2pole()
{
    if ((ui->spinBoxFrequency->value() > 0.0) && (ui->spinBoxFrequency->value() < 1.0)) {
        // update the z-plane filter with the user-defined parameters:
        double frequencyInRadians = M_PI * ui->spinBoxFrequency->value();
        double c = cos(frequencyInRadians * 0.5) / sin(frequencyInRadians * 0.5);
        // butterworth 2-pole-filter coefficients:
        std::complex<double> feedback[] = { c*c + c*sqrt(2.0) + 1.0, -c*c*2.0 + 2.0, c*c - c*sqrt(2.0) + 1.0 };
        std::complex<double> feedforward[] = { 1.0, 2.0, 1.0 };
    //    qDebug() << feedforward[0].real() << feedforward [1].real() << feedforward[2].real();
    //    qDebug() << feedback[0].real() << feedback[1].real() << feedback[2].real();
    //    qDebug() << "sum" << feedforward[0].real() + feedforward [1].real() + feedforward[2].real() - feedback[1].real() - feedback[2].real();
        // setup the z-plane filter:
        filter2pole.pole(0) = std::complex<double>((-feedback[1] - std::sqrt(feedback[1] * feedback[1] - 4.0 * feedback[2] * feedback[0])) / (2.0 * feedback[0]));
        filter2pole.pole(1) = std::complex<double>((-feedback[1] + std::sqrt(feedback[1] * feedback[1] - 4.0 * feedback[2] * feedback[0])) / (2.0 * feedback[0]));
        filter2pole.zero(0) = std::complex<double>((-feedforward[1] - std::sqrt(feedforward[1] * feedforward[1] - 4.0 * feedforward[2] * feedforward[0])) / (2.0 * feedforward[0]));
        filter2pole.zero(1) = std::complex<double>((-feedforward[1] + std::sqrt(feedforward[1] * feedforward[1] - 4.0 * feedforward[2] * feedforward[0])) / (2.0 * feedforward[0]));
    //    qDebug() << filter2pole.pole(0).real() << filter2pole.pole(0).imag();
    //    qDebug() << filter2pole.pole(1).real() << filter2pole.pole(1).imag();
    //    qDebug() << filter2pole.zero(0).real() << filter2pole.zero(0).imag();
    //    qDebug() << filter2pole.zero(1).real() << filter2pole.zero(1).imag();

        for (size_t i = 0; i < filter2pole.poleCount(); i++) {
            poleItems[i]->setPos(350 + 250 * filter2pole.pole(i).real(), 350 - 250 * filter2pole.pole(i).imag());
        }
        for (size_t i = 0; i < filter2pole.zeroCount(); i++) {
            zeroItems[i]->setPos(350 + 250 * filter2pole.zero(i).real(), 350 - 250 * filter2pole.zero(i).imag());
        }

        // compute the frequency response:
        for (int i = 0; i < squaredAmplitudeResponse2pole.size(); i++) {
            double frequencyInRadians = (double)i * M_PI / (double)(squaredAmplitudeResponse2pole.size() - 1);
            squaredAmplitudeResponse2pole[i] = std::norm(filter2pole.frequencyResponse(frequencyInRadians));
            if (squaredAmplitudeResponse2pole[i] == 0.0) {
                squaredAmplitudeResponse2pole[i] = -80.0;
            } else {
                squaredAmplitudeResponse2pole[i] = std::max(-80.0, ZPlaneFilter::convertPowerToDecibel(squaredAmplitudeResponse2pole[i]));
            }
        }
        // update the graphics view:
        for (int i = 0; i < squaredAmplitudeResponse2pole.size() - 1; i++) {
            lines2pole[i]->setLine(i * xscale, -squaredAmplitudeResponse2pole[i] * yscale, (i+1) * xscale, -squaredAmplitudeResponse2pole[i+1] * yscale);
        }
    }
}

void ZPlaneWidget::drawButterworthPoleTrail()
{
    QVector<std::complex<double> > poles;
    for (int i = 1; i < 99; i++) {
        double frequencyInRadians = M_PI * (double)i * 0.01;
        double c = cos(frequencyInRadians * 0.5) / sin(frequencyInRadians * 0.5);
        // butterworth 2-pole-filter coefficients:
        std::complex<double> feedback[] = { c*c + c*sqrt(2.0) + 1.0, -c*c*2.0 + 2.0, c*c - c*sqrt(2.0) + 1.0 };
        std::complex<double> pole((-feedback[1] - std::sqrt(feedback[1] * feedback[1] - 4.0 * feedback[2] * feedback[0])) / (2.0 * feedback[0]));
        poles.append(pole);
    }
    for (int i = 0; i < poles.size() - 1; i++) {
        ui->graphicsView->scene()->addLine(350 + 250 * poles[i].real(), 350 - 250 * poles[i].imag(), 350 + 250 * poles[i+1].real(), 350 - 250 * poles[i+1].imag(), QPen(Qt::red));
        ui->graphicsView->scene()->addLine(350 + 250 * poles[i].real(), 350 + 250 * poles[i].imag(), 350 + 250 * poles[i+1].real(), 350 + 250 * poles[i+1].imag(), QPen(Qt::green));
    }
}
