/*
    Copyright 2011 Arne Jacobs

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

#include "midipitchslider.h"

MidiPitchSlider::MidiPitchSlider(QWidget *parent) :
    QSlider(parent),
    channel(0),
    changingValue(false)
{
    // midi pitch wheel messages are in the range [-8192,8191]:
    setRange(0x0000, 0x3FFF);
    setValue(0x2000);
    // get notified of slider value changes:
    QObject::connect(this, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)));
}

unsigned char MidiPitchSlider::getChannel() const
{
    return channel;
}

void MidiPitchSlider::setChannel(unsigned char channel)
{
    this->channel = channel;
}

void MidiPitchSlider::onPitchWheel(unsigned char channel, unsigned int pitch)
{
    // check if the message has the right channel:
    if (channel == this->channel) {
        // set the slider accordingly:
        changingValue = true;
        setValue(pitch);
        changingValue = false;
    }
}

void MidiPitchSlider::mouseReleaseEvent(QMouseEvent * ev)
{
    // call the base class implementation:
    QSlider::mouseReleaseEvent(ev);
    // reset value to 0x2000:
    setValue(0x2000);
}

void MidiPitchSlider::onValueChanged(int value)
{
    // emit the corresponding signal:
    if (!changingValue) {
        pitchWheel(channel, value);
    }
}
