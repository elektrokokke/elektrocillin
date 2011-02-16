#ifndef WHITENOISEGENERATOR_H
#define WHITENOISEGENERATOR_H

#include "audioprocessor.h"

class WhiteNoiseGenerator : public AudioProcessor
{
public:
    WhiteNoiseGenerator(double sampleRate = 44100);

    void processAudio(const double *inputs, double *outputs, jack_nframes_t time);
};

#endif // WHITENOISEGENERATOR_H
