#include "midiparameterprocessor.h"

MidiParameterProcessor::MidiParameterProcessor() :
    channel(0)
{
    // register a parameter that controls the first controller used for controlling parameters:
    registerParameter("First MIDI controller", 0, 0, 127, 1);
}

void MidiParameterProcessor::processController(unsigned char, unsigned char controller, unsigned char value, jack_nframes_t time)
{
    int parameterId = (int)controller - qRound(getParameter(0).value) + 1;
    Q_ASSERT(parameterId > 0);
    if (parameterId < getNrOfParameters()) {
        // update the corresponding parameter value:
        const ParameterProcessor::Parameter &parameter = getParameter(parameterId);
        if (parameter.max != parameter.min) {
            ParameterProcessor::setParameterValue(parameterId, (double)value * (parameter.max - parameter.min) / 127.0 + parameter.min, time);
        }
    }
}

bool MidiParameterProcessor::setParameterValue(int index, double value, double min, double max, unsigned int time)
{
    // send a MIDI controller update if the value was changed:
    if (ParameterProcessor::setParameterValue(index, value, min, max, time)) {
        // first parameter is not controllable via MIDI:
        if (index) {
            int controller = index + qRound(getParameter(0).value) - 1;
            Q_ASSERT(controller >= 0);
            if (controller <= 127) {
                const ParameterProcessor::Parameter &parameter = getParameter(index);
                // parameters with equal min and max values are not controllable via MIDI:
                if (parameter.max != parameter.min) {
                    int controllerValue = qRound((parameter.value - parameter.min) * 127.0 / (parameter.max - parameter.min));
                    writeControlller(channel, controller, controllerValue, time);
                }
            }
        }
        return true;
    } else {
        return false;
    }
}
