#ifndef IIRMOOGFILTER_H
#define IIRMOOGFILTER_H

#include "iirfilter.h"
#include "midiprocessor.h"
#include "eventprocessor.h"
#include "parameterprocessor.h"
#include <QVector>

class IirMoogFilter : public IirFilter, public MidiProcessor, public ParameterProcessor
{
public:
    IirMoogFilter(double sampleRate = 44100, int zeros = 0);
    IirMoogFilter(const IirMoogFilter &tocopy);

    void setFrequencyController(unsigned char controller);
    unsigned char getFrequencyController() const;
    void setResonanceController(unsigned char controller);
    unsigned char getResonanceController() const;

    // reimplemented from IirFilter (originally from AudioProcessor):
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);
    // reimplemented from MidiProcessor:
    virtual void processNoteOn(unsigned char channel, unsigned char noteNumber, unsigned char velocity, jack_nframes_t time);
    virtual void processPitchBend(unsigned char channel, unsigned int value, jack_nframes_t time);
    virtual void processController(unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time);
    // reimplemented from ParameterProcessor:
    virtual int getNrOfParameters() const;
    virtual const ParameterProcessor::Parameter & getParameter(int index) const;
    virtual void setParameterValue(int index, double value);
    virtual bool hasParameterChanged(int index);

    double getBaseCutoffFrequency() const;
    double getResonance() const;
    double getCutoffMidiNoteOffset() const;
    double getCutoffAudioModulationIntensity() const;
    double getCutoffPitchBendModulationIntensity() const;
    double getCutoffControllerModulationIntensity() const;
    double getCutoffAudioModulation() const;
    double getCutoffControllerModulation() const;
    double getCutoffPitchBendModulation() const;
    double getResonanceAudioModulation() const;
    double getResonanceControllerModulation() const;

    void setSampleRate(double sampleRate);

    void computeCoefficients();
private:
    unsigned char frequencyController, resonanceController;
    QVector<ParameterProcessor::Parameter> parameters;
    QVector<bool> parametersChanged;
    bool recomputeCoefficients;
};

#endif // IIRMOOGFILTER_H
