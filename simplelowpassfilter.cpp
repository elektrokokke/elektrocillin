#include "simplelowpassfilter.h"

SimpleLowpassFilter::SimpleLowpassFilter(double a_, double b_) :
    previous(0.0),
    a(a_ / (a_ + b_)),
    b(b_ / (a_ + b_)),
    started(false)
{
}

double SimpleLowpassFilter::filter(double sample)
{
    if (!started) {
        started = true;
        previous = sample;
    }
    double filtered = a * sample + b * previous;
    previous = filtered;
    return filtered;
}

void SimpleLowpassFilter::reset()
{
    started = false;
}
