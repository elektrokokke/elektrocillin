#ifndef MULTIPLYPROCESSOR_H
#define MULTIPLYPROCESSOR_H

#include "audioprocessor.h"

class MultiplyProcessor : public AudioProcessor
{
public:
    MultiplyProcessor(double sampleRate = 44100);
    MultiplyProcessor(const QStringList &inputPortNames, double sampleRate = 44100);

    void processAudio(const double *inputs, double *outputs, jack_nframes_t time);
};

#endif // MULTIPLYPROCESSOR_H
