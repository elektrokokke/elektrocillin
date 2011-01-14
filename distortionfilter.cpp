#include "distortionfilter.h"
#include <cmath>

DistortionFilter::DistortionFilter(double distortion_) :
    distortion(distortion_)
{
}

double DistortionFilter::filter(double sample)
{
    if (sample > 0.0) {
        return 1.0 - pow(1.0 - sample, distortion);
    } else if (sample < 0.0) {
        return -1.0 + pow(sample + 1.0, distortion);
    } else {
        return 0.0;
    }
}

void DistortionFilter::setDistortion(double distortion)
{
    this->distortion = distortion;
}
