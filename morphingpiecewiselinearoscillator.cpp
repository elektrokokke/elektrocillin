#include "morphingpiecewiselinearoscillator.h"

MorphingPiecewiseLinearOscillator::MorphingPiecewiseLinearOscillator(const PiecewiseLinearOscillator *osc1_, const PiecewiseLinearOscillator *osc2_) :
    osc1(osc1_),
    osc2(osc2_),
    morph(0.0)
{
    Q_ASSERT(osc1->size() == osc2->size());
}

QPointF MorphingPiecewiseLinearOscillator::getNode(int index) const
{
    // compute weighted average of the two base oscillators:
    return (1.0 - morph) * osc1->getNode(index) + morph * osc2->getNode(index);
}

int MorphingPiecewiseLinearOscillator::size() const
{
    return osc1->size();
}

void MorphingPiecewiseLinearOscillator::setMorph(double morph)
{
    Q_ASSERT((morph >= 0.0) && (morph <= 1.0));
    this->morph = morph;
}

double MorphingPiecewiseLinearOscillator::getMorph() const
{
    return morph;
}
