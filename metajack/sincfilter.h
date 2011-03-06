#ifndef SINCFILTER_H
#define SINCFILTER_H

#include "audioprocessor.h"

class SincFilter : public AudioProcessor
{
public:
    /**
      @param size determines the number of filter coefficients. Actual filter size
        will be size * 2 + 1.
      */
    SincFilter(int size, double frequency, double sampleRate);

    void feed(double input);
    double process();

    /**
      A short cut for feed() followed by process().
      */
    void processAudio(const double *inputs, double *outputs, jack_nframes_t time);
private:
    int index;
    QVector<double> coefficients, previousInputs;
};

#endif // SINCFILTER_H
