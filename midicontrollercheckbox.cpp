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
