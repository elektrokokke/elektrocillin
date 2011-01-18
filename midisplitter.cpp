#include "midisplitter.h"

ControlChange::ControlChange(QObject *parent) :
    QObject(parent)
{
}

void ControlChange::onControlChange(unsigned char channel, unsigned char controller, unsigned char value)
{
    receivedControlChange(channel, controller, value);
}

MidiSplitter::MidiSplitter(QObject *parent) :
    QObject(parent)
{
}

ControlChange * MidiSplitter::getControlChangeSink(unsigned char controller)
{
    // create a new sink, if necessary:
    if (!mapControllerToControlChange.contains(controller)) {
        mapControllerToControlChange.insert(controller, new ControlChange(this));
    }
    return mapControllerToControlChange.value(controller);
}

void MidiSplitter::splitControlChangeByController(unsigned char channel, unsigned char controller, unsigned char value)
{
    // get the corresponding ControlChange object:
    ControlChange *controlChange = mapControllerToControlChange.value(controller, 0);
    if (controlChange) {
        controlChange->onControlChange(channel, controller, value);
    }
}
