#include "monophonicsynthesizer.h"
#include <cmath>

MonophonicSynthesizer::MonophonicSynthesizer() :
    pulseOsc1(0.1),
    pulseOsc2(2.0 * M_PI - 0.1),
    sawOsc1(0.1),
    sawOsc2(M_PI - 0.1),
    morphOsc1(&pulseOsc1, &pulseOsc2),
    morphOsc2(&sawOsc1, &sawOsc2),
    morphOsc3(&morphOsc1, &morphOsc2),
    lfo(false),
    lfo2(false),
    lfo3(false),
    envelope(0.001, 0.01, 0.5, 0.5)
{
    lfo.setFrequency(99.0);
    lfo2.setFrequency(99.1);
    lfo3.setFrequency(99.2);
}

void MonophonicSynthesizer::setSampleRate(double sampleRate)
{
    AudioSource::setSampleRate(sampleRate);
    pulseOsc1.setSampleRate(sampleRate);
    pulseOsc2.setSampleRate(sampleRate);
    sawOsc1.setSampleRate(sampleRate);
    sawOsc2.setSampleRate(sampleRate);
    morphOsc1.setSampleRate(sampleRate);
    morphOsc2.setSampleRate(sampleRate);
    morphOsc3.setSampleRate(sampleRate);
    lfo.setSampleRate(sampleRate);
    lfo2.setSampleRate(sampleRate);
    lfo3.setSampleRate(sampleRate);
    envelope.setSampleRate(sampleRate);
}

void MonophonicSynthesizer::setFrequency(double frequency)
{
    pulseOsc1.setFrequency(frequency);
    pulseOsc2.setFrequency(frequency);
    sawOsc1.setFrequency(frequency);
    sawOsc2.setFrequency(frequency);
    morphOsc1.setFrequency(frequency);
    morphOsc2.setFrequency(frequency);
    morphOsc3.setFrequency(frequency);
    lfo.setFrequency(morphOsc3.getFrequency() * 0.25 - 1.0);
    lfo2.setFrequency(morphOsc3.getFrequency() * 0.25 - 2.0);
    lfo3.setFrequency(morphOsc3.getFrequency() * 0.25 - 3.0);
}

void MonophonicSynthesizer::pushNote(unsigned char midiNoteNumber)
{
    midiNoteNumbers.push(midiNoteNumber);
    frequencies.push(computeFrequencyFromMidiNoteNumber(midiNoteNumber));
    setFrequency(frequencies.top());
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
                setFrequency(frequencies.top());
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

double MonophonicSynthesizer::nextSample()
{
    // get level from ADSR envelope:
    double envelopeLevel = envelope.nextSample();
    if (envelopeLevel) {
        morphOsc1.setMorph(0.5 * lfo.nextSample() + 0.5);
        morphOsc2.setMorph(0.5 * lfo2.nextSample() + 0.5);
        morphOsc3.setMorph(0.5 * lfo3.nextSample() + 0.5);
    }
    double oscillatorLevel = morphOsc3.nextSample();
    return filter.filter(envelopeLevel * oscillatorLevel);
}

double MonophonicSynthesizer::computeFrequencyFromMidiNoteNumber(unsigned char midiNoteNumber)
{
    // 440 Hz is note number 69:
    double octave = ((double)midiNoteNumber - 69.0) / 12.0;
    double frequency = 440.0 * pow(2.0, octave);
    return frequency;
}
