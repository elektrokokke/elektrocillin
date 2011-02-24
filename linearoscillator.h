#ifndef LINEAROSCILLATOR_H
#define LINEAROSCILLATOR_H

#include "oscillator.h"
#include "jackringbuffer.h"
#include "linearintegralinterpolator.h"
#include "cubicsplineinterpolator.h"
#include <QQueue>
#include <QPair>

class LinearOscillator : public Oscillator
{
public:
    class ChangeControlPointEvent : public RingBufferEvent
    {
    public:
        int index;
        double x, y;
    };

    class ChangeAllControlPointsEvent : public RingBufferEvent
    {
    public:
        QVector<double> xx, yy;
    };

    LinearOscillator(double frequencyModulationIntensity = 2.0/12.0, double sampleRate = 44100, const QStringList &additionalInputPortNames = QStringList(), int sincWindowSize = 4);
    /**
      @param interpolator has to span the interval [0..2 * pi]
      */
    LinearOscillator(const LinearInterpolator &interpolator, double frequencyModulationIntensity = 2.0/12.0, double sampleRate = 44100, int sincWindowSize = 8);

    const LinearInterpolator & getLinearInterpolator() const;
    void setLinearInterpolator(const LinearInterpolator &interpolator);

    virtual void processEvent(const ChangeControlPointEvent *event, jack_nframes_t time);
    virtual void processEvent(const ChangeAllControlPointsEvent *event, jack_nframes_t time);
protected:
    double valueAtPhase(double phase);

private:
    LinearInterpolator interpolator;
    int sincWindowSize;
    CubicSplineInterpolator siInterpolator;

    void initializeSiInterpolator();
};

#endif // LINEAROSCILLATOR_H
