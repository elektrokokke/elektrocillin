#include "morphingpiecewiselinearoscillator.h"

MorphingPiecewiseLinearOscillator::MorphingPiecewiseLinearOscillator(const PiecewiseLinearOscillator *osc1_, const PiecewiseLinearOscillator *osc2_) :
    osc1(osc1_),
    osc2(osc2_),
    morph(0.0)
{
    Q_ASSERT(osc1->getTimes().size() == osc2->getTimes().size());
    for (int segmentIndex = 0; segmentIndex < osc1->getTimes().size(); segmentIndex++) {
        addNode(osc1->getTimes().value(segmentIndex), osc1->getValues().value(segmentIndex));
    }
}

void MorphingPiecewiseLinearOscillator::setMorph(double morph)
{
    Q_ASSERT((morph >= 0.0) && (morph <= 1.0));
    this->morph = morph;
    // interpolate between the two oscillator states:
    for (int segmentIndex = 0; segmentIndex < getTimes().size(); segmentIndex++) {
        getTimes()[segmentIndex] = (1.0 - morph) * osc1->getTimes().value(segmentIndex) + morph * osc2->getTimes().value(segmentIndex);
        getValues()[segmentIndex] = (1.0 - morph) * osc1->getValues().value(segmentIndex) + morph * osc2->getValues().value(segmentIndex);
    }
}

double MorphingPiecewiseLinearOscillator::getMorph() const
{
    return morph;
}
