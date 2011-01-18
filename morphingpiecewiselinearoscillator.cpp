#include "morphingpiecewiselinearoscillator.h"

MorphingPiecewiseLinearOscillator::MorphingPiecewiseLinearOscillator(const PiecewiseLinearOscillator *osc1_, const PiecewiseLinearOscillator *osc2_) :
    osc1(osc1_),
    osc2(osc2_),
    morph(0.0)
{
    Q_ASSERT(osc1->getNodes().size() == osc2->getNodes().size());
    for (int segmentIndex = 0; segmentIndex < osc1->getNodes().size(); segmentIndex++) {
        addNode(osc1->getNodes().at(segmentIndex));
    }
}

void MorphingPiecewiseLinearOscillator::setMorph(double morph)
{
    Q_ASSERT((morph >= 0.0) && (morph <= 1.0));
    this->morph = morph;
    // compute weighted average of the two base oscillators:
    for (int segmentIndex = 0; segmentIndex < getNodes().size(); segmentIndex++) {
        getNodes()[segmentIndex] = (1.0 - morph) * osc1->getNodes().at(segmentIndex) + morph * osc2->getNodes().at(segmentIndex);
    }
}

double MorphingPiecewiseLinearOscillator::getMorph() const
{
    return morph;
}
