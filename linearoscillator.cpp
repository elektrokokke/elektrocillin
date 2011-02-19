#include "linearoscillator.h"
#include "cisi.h"
#include <cmath>
#include <QDebug>

LinearOscillator::LinearOscillator(double frequencyModulationIntensity, double sampleRate, const QStringList &additionalInputPortNames, int sincWindowSize) :
    Oscillator(frequencyModulationIntensity, sampleRate, additionalInputPortNames),
    interpolator(QVector<double>(1), QVector<double>(1)),
    integral(interpolator),
    sineIntegral(QVector<double>(0), QVector<double>(0), QVector<double>(0))
{
    interpolator.getX()[0] = 0;
    interpolator.getY()[0] = -1;
    interpolator.getX().append(2 * M_PI);
    interpolator.getY().append(1);
    integral = LinearIntegralInterpolator(interpolator);
    initializeSineIntegral(sincWindowSize);
}

LinearOscillator::LinearOscillator(const LinearInterpolator &interpolator_, double frequencyModulationIntensity, double sampleRate, int sincWindowSize) :
    Oscillator(frequencyModulationIntensity, sampleRate),
    interpolator(interpolator_),
    integral(interpolator_),
    sineIntegral(QVector<double>(0), QVector<double>(0), QVector<double>(0))
{
    initializeSineIntegral(sincWindowSize);
}

const LinearInterpolator & LinearOscillator::getLinearInterpolator() const
{
    return interpolator;
}

void LinearOscillator::setLinearInterpolator(const LinearInterpolator &interpolator)
{
    this->interpolator = interpolator;
    integral = LinearIntegralInterpolator(interpolator);
}

const LinearIntegralInterpolator & LinearOscillator::getLinearIntegralInterpolator() const
{
    return integral;
}

double LinearOscillator::valueAtPhase(double phase, double previousPhase)
{
    // remove the oldest value from the queue:
    integralValuesQueue.dequeue();
    // calculate the news value and add it to the queue:
    if (phase == previousPhase) {
        QList<QPair<double, double> > list;
        list.append(QPair<double, double>(0, 1));
        integralValuesQueue.enqueue(list);
    } else {
        QList<QPair<double, double> > list;
        double phaseDifference = phase - previousPhase;
        int previousIndex, index;
        double previousIntegralValue = integral.evaluate(previousPhase, &previousIndex);
        double integralValue = integral.evaluate(phase, &index);
        for (int i = previousIndex; i != index; ) {
            double intermediatePhase = integral.getX()[i + 1];
            if (intermediatePhase != previousPhase) {
                double intermediateIntegralValue = integral.interpolate(i, intermediatePhase);
                list.append(QPair<double, double>((intermediateIntegralValue - previousIntegralValue) / (intermediatePhase - previousPhase), (intermediatePhase - previousPhase) / phaseDifference));
                previousPhase = intermediatePhase;
                previousIntegralValue = intermediateIntegralValue;
            }
            i++;
            if (i == integral.getX().size() - 1) {
                i = 0;
                previousPhase = integral.getX()[0];
                previousIntegralValue = integral.interpolate(0, previousPhase);
            }
        }
        if (phase != previousPhase) {
            list.append(QPair<double, double>((integralValue - previousIntegralValue) / (phase - previousPhase), (phase - previousPhase) / phaseDifference));
        }
        integralValuesQueue.enqueue(list);
    }
    // now band-limit the signal using the sinc function:
    double output = 0;
    for (int i = 0; i < integralValuesQueue.size(); i++) {
        double x = sineIntegral.getX()[i];
        double previousSincIntegralValue = sineIntegral.evaluate(x);
        QList<QPair<double, double> > &list = integralValuesQueue[i];
        for (int j = 0; j < list.size(); j++) {
            double oscillatorIntegralValue = list[j].first;
            x += list[j].second;
            double sincIntegralValue = sineIntegral.evaluate(x);
            output += oscillatorIntegralValue * (sincIntegralValue - previousSincIntegralValue);
            previousSincIntegralValue = sincIntegralValue;
        }
    }
    return output;
}

void LinearOscillator::initializeSineIntegral(int sincWindowSize)
{
    // create a cubic spline that approximates the cardinal sine integral:
    QVector<double> xx, yy;
    for (int i = -sincWindowSize; i <= sincWindowSize; i++) {
        double x = M_PI * i;
        std::complex<double> cs;
        cisi(x, cs);
        xx.append(i);
        yy.append(cs.imag() / M_PI);
        if (i < sincWindowSize) {
            QList<QPair<double, double> > list;
            list.append(QPair<double, double>(0, 1));
            integralValuesQueue.append(list);
        }
    }
    sineIntegral = CubicSplineInterpolator(xx, yy);
}
