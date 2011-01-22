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
