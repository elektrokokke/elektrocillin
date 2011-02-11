#include "multiplyprocessor.h"

MultiplyProcessor::MultiplyProcessor(double sampleRate) :
    AudioProcessor(QStringList("Factor 1") + QStringList("Factor 2"), QStringList("Product"), sampleRate)
{
}

MultiplyProcessor::MultiplyProcessor(const QStringList &inputPortNames, double sampleRate) :
    AudioProcessor(inputPortNames, QStringList("Product"), sampleRate)
{
}

void MultiplyProcessor::processAudio(const double *inputs, double *outputs, jack_nframes_t)
{
    // multiply all inputs:
    double product = 1.0;
    for (int i = 0; i < getNrOfInputs(); i++) {
        product *= inputs[i];
    }
    outputs[0] = product;
}
