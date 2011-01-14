#ifndef DISTORTIONFILTER_H
#define DISTORTIONFILTER_H

class DistortionFilter
{
public:
    DistortionFilter(double distortion = 2.0);

    double filter(double sample);
    void setDistortion(double distortion);

private:
    double distortion;
};

#endif // DISTORTIONFILTER_H
