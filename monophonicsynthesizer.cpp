#include "monophonicsynthesizer.h"
#include <cmath>

MonophonicSynthesizer::MonophonicSynthesizer() :
    frequency(0.0),
    pitchBendFactor(1.0),
    morph(0.0),
    osc1(M_PI),
    osc2(0.1),
    morphOsc1(&osc1, &osc2),
    envelope(0.01, 0.01, 0.75, 0.5),
    filterAudio(0.5),
    filterMorph(0.001)
{
    morphOsc1.setMorph(0.5);
}

void MonophonicSynthesizer::setSampleRate(double sampleRate)
{
    AudioSource::setSampleRate(sampleRate);
    osc1.setSampleRate(sampleRate);
    osc2.setSampleRate(sampleRate);
    morphOsc1.setSampleRate(sampleRate);
    envelope.setSampleRate(sampleRate);
}

void MonophonicSynthesizer::setFrequency(double frequency, double pitchBendFactor)
{
    this->frequency = frequency;
    this->pitchBendFactor = pitchBendFactor;
    osc1.setFrequency(frequency * pitchBendFactor);
    osc2.setFrequency(frequency * pitchBendFactor);
    morphOsc1.setFrequency(frequency * pitchBendFactor);
}

void MonophonicSynthesizer::pushNote(unsigned char midiNoteNumber)
{
    midiNoteNumbers.push(midiNoteNumber);
    frequencies.push(computeFrequencyFromMidiNoteNumber(midiNoteNumber));
    setFrequency(frequencies.top(), pitchBendFactor);
    // (re-)trigger the ADSR envelope:
    envelope.noteOn();
}

void MonophonicSynthesizer::popNote(unsigned char midiNoteNumber)
{
    if (!midiNoteNumbers.isEmpty()) {
        // test if this is the topmost note:
        if (midiNoteNumbers.top() == midiNoteNumber) {
            midiNoteNumbers.pop();
            frequencies.pop();
            if (midiNoteNumbers.isEmpty()) {
                // enter the release phase:
                envelope.noteOff();
            } else {
                setFrequency(frequencies.top(), pitchBendFactor);
                // retrigger the ADSR envelope:
                envelope.noteOn();
            }
        } else {
            // the note has to be removed from somewhere in the stack
            // (current playing note remains unchanged):
            int index = midiNoteNumbers.lastIndexOf(midiNoteNumber);
            if (index != -1) {
                midiNoteNumbers.remove(index);
                frequencies.remove(index);
            }
        }
    }
}

void MonophonicSynthesizer::setMidiPitch(unsigned int pitch)
{
    setFrequency(frequency, computePitchBendFactorFromMidiPitch(pitch));
}

void MonophonicSynthesizer::setController(unsigned char, unsigned char value)
{
    morph = (double)value / 127.0;
}

double MonophonicSynthesizer::nextSample()
{
    // get level from ADSR envelope:
    double envelopeLevel = envelope.nextSample();
    morphOsc1.setMorph(filterMorph.filter(morph));
    double oscillatorLevel = morphOsc1.nextSample();
    return filterAudio.filter(envelopeLevel * oscillatorLevel);
}

double MonophonicSynthesizer::computeFrequencyFromMidiNoteNumber(unsigned char midiNoteNumber)
{
    // 440 Hz is note number 69:
    double octave = ((double)midiNoteNumber - 69.0) / 12.0;
    double frequency = 440.0 * pow(2.0, octave);
    return frequency;
}

double MonophonicSynthesizer::computePitchBendFactorFromMidiPitch(unsigned int pitch)
{
    // center it around 0x2000:
    int pitchCentered = (int)pitch - 0x2000;
    // -8192 means minus two half tones => -49152 is one octave => factor 2^(-1)
    // +8192 means plus two half tones => +49152 is one octave => factor 2^1
    return pow(2.0, (double)pitchCentered / 49152.0);
}
