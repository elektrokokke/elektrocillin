#ifndef LINEAROSCILLATOR_H
#define LINEAROSCILLATOR_H

#include "oscillator.h"
#include "linearintegralinterpolator.h"
#include "cubicsplineinterpolator.h"
#include <QQueue>
#include <QPair>

class LinearOscillator : public Oscillator
{
public:
    LinearOscillator(double frequencyModulationIntensity = 2.0/12.0, double sampleRate = 44100, const QStringList &additionalInputPortNames = QStringList(), int sincWindowSize = 8);
    /**
      @param interpolator has to span the interval [0..2 * pi]
      */
    LinearOscillator(const LinearInterpolator &interpolator, double frequencyModulationIntensity = 2.0/12.0, double sampleRate = 44100, int sincWindowSize = 8);

    const LinearInterpolator & getLinearInterpolator() const;
    void setLinearInterpolator(const LinearInterpolator &interpolator);
    const LinearIntegralInterpolator & getLinearIntegralInterpolator() const;

protected:
    double valueAtPhase(double phase, double previousPhase);

private:
    LinearInterpolator interpolator;
    LinearIntegralInterpolator integral;
    CubicSplineInterpolator sineIntegral;
    QQueue<QList<QPair<double, double> > > integralValuesQueue;

    void initializeSineIntegral(int sincWindowSize);
};

#endif // LINEAROSCILLATOR_H
