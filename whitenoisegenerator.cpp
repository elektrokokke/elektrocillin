#include "whitenoisegenerator.h"
#include <cstdlib>

WhiteNoiseGenerator::WhiteNoiseGenerator(double sampleRate) :
    AudioProcessor(QStringList(), QStringList("Noise out"), sampleRate)
{
}

void WhiteNoiseGenerator::processAudio(const double *, double *outputs, jack_nframes_t)
{
    int randomNumber = rand();
    outputs[0] = (double)randomNumber / (double)RAND_MAX * 2.0 - 1.0;
}
