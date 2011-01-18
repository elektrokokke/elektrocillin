#ifndef MORPHINGPIECEWISELINEAROSCILLATOR_H
#define MORPHINGPIECEWISELINEAROSCILLATOR_H

#include "piecewiselinearoscillator.h"

class MorphingPiecewiseLinearOscillator : public PiecewiseLinearOscillator
{
public:
    MorphingPiecewiseLinearOscillator(const PiecewiseLinearOscillator *osc1, const PiecewiseLinearOscillator *osc2);

    virtual QPointF getNode(int index) const;
    virtual int size() const;

    void setMorph(double morph);
    double getMorph() const;

private:
    const PiecewiseLinearOscillator *osc1, *osc2;
    double morph;
};

#endif // MORPHINGPIECEWISELINEAROSCILLATOR_H
