#ifndef BUTTERWORTH2POLEFILTER_H
#define BUTTERWORTH2POLEFILTER_H

#include "zplanefilter.h"

class Butterworth2PoleFilter : public ZPlaneFilter
{
public:
    Butterworth2PoleFilter(double normalizedCutoffFrequency);
};

#endif // BUTTERWORTH2POLEFILTER_H
