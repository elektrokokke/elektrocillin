#ifndef IIRMOOGFILTER_H
#define IIRMOOGFILTER_H

#include "iirfilter.h"

class IirMoogFilter : public IirFilter
{
public:
    struct Parameters {
        double frequency;
        double frequencyOffsetFactor;
        double frequencyPitchBendFactor;
        double frequencyModulationFactor;
        double frequencyModulationIntensity;
        double resonance;
    };

    IirMoogFilter(double sampleRate = 44100, int zeros = 0);

    // reimplemented from IIRFilter:
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);
    // reimplemented from MidiProcessor:
    virtual void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    virtual void processPitchBend(unsigned char channel, unsigned int value, jack_nframes_t time);
    virtual void processController(unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time);

    void setSampleRate(double sampleRate);

    void setParameters(const Parameters parameters);
    const Parameters & getParameters() const;

    void computeCoefficients();
private:
    Parameters parameters;
};

#endif // IIRMOOGFILTER_H
