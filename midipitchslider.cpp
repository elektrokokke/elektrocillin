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
