/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Elektrocillin is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Elektrocillin.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "midicontrollerslider.h"
#include <QDebug>

MidiControllerSlider::MidiControllerSlider(QWidget *parent) :
    QSlider(parent),
    channel(0),
    controller(0),
    changingValue(false)
{
    // midi controller messages are in the range [0,127]:
    setRange(0, 127);
    // get notified of slider value changes:
    QObject::connect(this, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)));
}

unsigned char MidiControllerSlider::getChannel() const
{
    return channel;
}

unsigned char MidiControllerSlider::getController() const
{
    return controller;
}

void MidiControllerSlider::setChannel(unsigned char channel)
{
    this->channel = channel;
}

void MidiControllerSlider::setController(unsigned char controller)
{
    this->controller = controller;
}

void MidiControllerSlider::onControlChange(unsigned char channel, unsigned char controller, unsigned char value)
{
    // check if the control change has the right channel and controller number:
    if ((channel == this->channel) && (controller == this->controller)) {
        //qDebug() << "controller value changed to" << value;
        // set the slider accordingly:
        changingValue = true;
        setValue(value);
        changingValue = false;
    }
}

void MidiControllerSlider::onValueChanged(int value)
{
    // emit the corresponding signal:
    if (!changingValue) {
        controlChanged(channel, controller, value);
    }
}
