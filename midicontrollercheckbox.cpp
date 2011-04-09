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

#include "midicontrollercheckbox.h"

MidiControllerCheckBox::MidiControllerCheckBox(QWidget *parent) :
    QCheckBox(parent),
    channel(0),
    controller(0),
    changingValue(false)
{
    // get notified of checkbox value changes:
    QObject::connect(this, SIGNAL(toggled(bool)), this, SLOT(onToggled(bool)));
}

unsigned char MidiControllerCheckBox::getChannel() const
{
  return channel;
}

unsigned char MidiControllerCheckBox::getController() const
{
  return controller;
}

void MidiControllerCheckBox::setChannel(unsigned char channel)
{
  this->channel = channel;
}

void MidiControllerCheckBox::setController(unsigned char controller)
{
  this->controller = controller;
}

void MidiControllerCheckBox::onControlChange(unsigned char channel, unsigned char controller, unsigned char value)
{
  // check if the control change has the right channel and controller number:
  if ((channel == this->channel) && (controller == this->controller)) {
      // set the slider accordingly:
      changingValue = true;
      setChecked(value >= 64);
      changingValue = false;
  }
}

void MidiControllerCheckBox::onToggled(bool checked)
{
  // emit the corresponding signal:
  if (!changingValue) {
      controlChanged(channel, controller, checked ? 127 : 0);
  }
}
