#include "midiparameterprocessor.h"

MidiParameterProcessor::MidiParameterProcessor() :
    channel(1)
{
}

void MidiParameterProcessor::processController(unsigned char, unsigned char controller, unsigned char value, jack_nframes_t time)
{
    if (controller < getNrOfParameters()) {
        // update the corresponding parameter value:
        const ParameterProcessor::Parameter &parameter = getParameter(controller);
        ParameterProcessor::setParameterValue(controller, (double)value * (parameter.max - parameter.min) / 127.0 + parameter.min, time);
    }
}

bool MidiParameterProcessor::setParameterValue(int index, double value, unsigned int time)
{
    // send a MIDI controller update if the value was changed:
    if (ParameterProcessor::setParameterValue(index, value, time)) {
        const ParameterProcessor::Parameter &parameter = getParameter(index);
        if (parameter.max != parameter.min) {
            int controllerValue = qRound((value - parameter.min) * 127.0 / (parameter.max - parameter.min));
            writeControlChange(channel, index, controllerValue, time);
        }
        return true;
    } else {
        return false;
    }
}
