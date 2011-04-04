#ifndef LINEARMORPHOSCILLATOR_H
#define LINEARMORPHOSCILLATOR_H

/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "polynomialoscillator.h"
#include "linearinterpolator.h"

class LinearMorphOscillator : public PolynomialOscillator
{
public:
    class ChangeControlPointEvent : public InterpolatorProcessor::ChangeControlPointEvent
    {
    public:
        ChangeControlPointEvent(int state_, int index_, double x_, double y_) :
            InterpolatorProcessor::ChangeControlPointEvent(index_, x_, y_),
            state(state_)
        {}
        int state;
    };

    LinearMorphOscillator(const LinearInterpolator &state1, const LinearInterpolator &state2);

    const LinearInterpolator & getState(int state) const;
    void setState(int state, const LinearInterpolator &interpolator);

    // reimplemented from Oscillator, to make the morph value controllable by MIDI:
    virtual void processController(unsigned char channel, unsigned char controller, unsigned char value, jack_nframes_t time);
    // reimplemented from Oscillator, to control morph by audio input:
    virtual void processAudio(const double *inputs, double *outputs, jack_nframes_t time);
    // reimplemented from LinearOscillator:
    virtual bool processEvent(const RingBufferEvent *event, jack_nframes_t time);
private:
    LinearInterpolator state[2];
    double morphAudio, morphMidi;

    void computeMorphedState();
};

#endif // LINEARMORPHOSCILLATOR_H
