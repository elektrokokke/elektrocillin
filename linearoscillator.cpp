#include "linearoscillator.h"
#include "cisi.h"
#include <cmath>
#include <QDebug>

LinearOscillator::LinearOscillator(double frequencyModulationIntensity, double sampleRate, const QStringList &additionalInputPortNames, int sincWindowSize) :
    Oscillator(frequencyModulationIntensity, sampleRate, additionalInputPortNames),
    interpolator(QVector<double>(1), QVector<double>(1)),
    integral(interpolator),
    sincIntegralValues(sincWindowSize * 2, 0)
{
    interpolator.getX()[0] = 0;
    interpolator.getY()[0] = -1;
    interpolator.getX().append(2 * M_PI);
    interpolator.getY().append(1);
    integral = LinearIntegralInterpolator(interpolator);
    for (int i = 0; i < sincWindowSize * 2; i++) {
        integralValuesQueue.append(0);
    }
    // compute the integral of the sinc function between multiples of PI:
    double previousSincIntegralValue = 0;
    double sum = 0;
    for (int i = 0; i < sincWindowSize; i++) {
        std::complex<double> cs;
        cisi(M_PI * (i + 1), cs);
        double sincIntegralValue = cs.imag();
        sincIntegralValues[sincWindowSize + i] = sincIntegralValues[sincWindowSize - i - 1] = sincIntegralValue - previousSincIntegralValue;
        qDebug() << sincIntegralValues[sincWindowSize + i];
        sum += sincIntegralValues[sincWindowSize + i] * 2;
        previousSincIntegralValue = sincIntegralValue;
    }
    for (int i = 0; i < sincWindowSize * 2; i++) {
        sincIntegralValues[i] /= sum;
    }
    qDebug() << sum;
}

LinearOscillator::LinearOscillator(const LinearInterpolator &interpolator_, double frequencyModulationIntensity, double sampleRate) :
    Oscillator(frequencyModulationIntensity, sampleRate),
    interpolator(interpolator_),
    integral(interpolator_)
{
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
        integralValuesQueue.enqueue(0);
    } else {
        double previousIntegralValue = integral.evaluate(previousPhase);
        // compare current phase with previous phase:
        if (phase < previousPhase) {
            // phase has wrapped around, adjust the previous integral level accordingly:
            previousIntegralValue -= integral.interpolate(integral.getX().size() - 2, integral.getX().back());
            previousPhase -= 2 * M_PI;
        }
        double integralValue = integral.evaluate(phase);
        // integral difference / phase difference is the oscillator output:
        integralValuesQueue.enqueue((integralValue - previousIntegralValue) / (phase - previousPhase));
    }
    // now band-limit the signal using the sinc function:
    double output = 0;
    for (int i = 0; i < sincIntegralValues.size(); i++) {
        output += sincIntegralValues[i] * integralValuesQueue[i];
    }
    return output;
}
