#ifndef PIECEWISELINEAROSCILLATOR_H
#define PIECEWISELINEAROSCILLATOR_H

#include "oscillator.h"
#include <QList>
#include <QPointF>

class PiecewiseLinearOscillator : public Oscillator
{
public:
    PiecewiseLinearOscillator(double frequencyModulationIntensity = 2.0/12.0, double sampleRate = 44100);
    virtual QPointF getNode(int index) const;
    virtual int size() const;

protected:
    double valueAtPhase(double phase);
    void addNode(const QPointF &node);

private:
    QList<QPointF> nodes;

    static double interpolate(const QPointF &n0, const QPointF &n1, double x);
};

#endif // PIECEWISELINEAROSCILLATOR_H
