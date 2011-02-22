#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include "midiprocessor.h"
#include "linearinterpolator.h"

class Oscillator : public MidiProcessor
{
public:
    Oscillator(double frequencyModulationIntensity = 2.0/12.0, double sampleRate = 44100, const QStringList &additionalInputPortNames = QStringList());

    void setDetuneController(unsigned char controller);
    unsigned char getDetuneController() const;

    virtual void setSampleRate(double sampleRate);
    virtual void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    virtual void processPitchBend(unsigned char channel, unsigned int value, jack_nframes_t time);
    virtual void processController(unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time);
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);

    void setFrequency(double hertz);
    double getFrequency() const;
    void setDetune(double cents);
    double getDetune() const;
    double getNormalizedAngularFrequency() const;

protected:
    virtual double valueAtPhase(double phase);

private:
    unsigned char detuneController;
    double frequency, frequencyDetuneFactor, detuneInCents, frequencyPitchBendFactor, frequencyModulationFactor, frequencyModulationIntensity, phase, normalizedAngularFrequency;

    void computeNormalizedAngularFrequency();
};

#endif // OSCILLATOR_H
