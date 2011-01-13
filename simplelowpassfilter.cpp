#include "simplelowpassfilter.h"

SimpleLowpassFilter::SimpleLowpassFilter(double a_, double b_) :
    previous(0.0),
    a(a_ / (a_ + b_)),
    b(b_ / (a_ + b_))
{
}

double SimpleLowpassFilter::filter(double sample)
{
    double filtered = a * sample + b * previous;
    previous = filtered;
    return filtered;
}
