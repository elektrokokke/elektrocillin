#ifndef FREQUENCYRESPONSE_H
#define FREQUENCYRESPONSE_H

class FrequencyResponse
{
public:
    virtual double getSquaredAmplitudeResponse(double frequencyInHertz) = 0;
};

#endif // FREQUENCYRESPONSE_H
