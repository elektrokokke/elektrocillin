#ifndef LINEAROSCILLATOR_H
#define LINEAROSCILLATOR_H

#include "oscillator.h"
#include "linearintegralinterpolator.h"
#include <QQueue>
#include <QVector>

class LinearOscillator : public Oscillator
{
public:
    LinearOscillator(double frequencyModulationIntensity = 2.0/12.0, double sampleRate = 44100, const QStringList &additionalInputPortNames = QStringList(), int sincWindowSize = 16);
    /**
      @param interpolator has to span the interval [0..2 * pi]
      */
    LinearOscillator(const LinearInterpolator &interpolator, double frequencyModulationIntensity = 2.0/12.0, double sampleRate = 44100);

    const LinearInterpolator & getLinearInterpolator() const;
    void setLinearInterpolator(const LinearInterpolator &interpolator);
    const LinearIntegralInterpolator & getLinearIntegralInterpolator() const;

protected:
    double valueAtPhase(double phase, double previousPhase);

private:
    LinearInterpolator interpolator;
    LinearIntegralInterpolator integral;
    QQueue<double> integralValuesQueue;
    QVector<double> sincIntegralValues;
};

#endif // LINEAROSCILLATOR_H
