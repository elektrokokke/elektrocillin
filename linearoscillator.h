#ifndef LINEAROSCILLATOR_H
#define LINEAROSCILLATOR_H

#include "oscillator.h"
#include "linearinterpolator.h"
#include "cubicsplineinterpolator.h"

class LinearOscillator : public Oscillator
{
public:
    LinearOscillator(double frequencyModulationIntensity = 2.0/12.0, double sampleRate = 44100, const QStringList &additionalInputPortNames = QStringList(), int sincWindowSize = 4);
    /**
      @param interpolator has to span the interval [0..2 * pi]
      */
    LinearOscillator(const LinearInterpolator &interpolator, double frequencyModulationIntensity = 2.0/12.0, double sampleRate = 44100, int sincWindowSize = 8);

    const LinearInterpolator & getLinearInterpolator() const;
    void setLinearInterpolator(const LinearInterpolator &interpolator);

    void changeControlPoints(const QVector<double> &xx, const QVector<double> &yy);
    // Reimplemented from Oscillator:
    virtual bool processEvent(const RingBufferEvent *event, jack_nframes_t time);
protected:
    double valueAtPhase(double phase);

private:
    LinearInterpolator interpolator;
    int sincWindowSize;
    CubicSplineInterpolator siInterpolator;

    void initializeSiInterpolator();
};

#endif // LINEAROSCILLATOR_H
