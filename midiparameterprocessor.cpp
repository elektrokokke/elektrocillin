#include "midiparameterprocessor.h"

MidiParameterProcessor::MidiParameterProcessor(const QStringList &additionMidiInputPortNames, const QStringList &additionMidiOutputPortNames, MidiProcessor::MidiWriter *midiWriter) :
    MidiProcessor(QStringList("Midi control in") + additionMidiInputPortNames, QStringList("Midi control out") + additionMidiOutputPortNames, midiWriter),
    channel(0)
{
    // register a parameter that controls the first controller used for controlling parameters:
    QMap<double, QString> stringValues;
    stringValues[-1] = "Learn";
    registerParameter("First MIDI controller", 0, -1, 127, 1, stringValues);
}

void MidiParameterProcessor::processController(int inputIndex, unsigned char, unsigned char controller, unsigned char value, jack_nframes_t time)
{
    if (inputIndex == 0) {
        int firstMidiController = qRound(getParameter(0).value);
        if (firstMidiController < 0) {
            // MIDI learn function; set the parameter from incoming controller message:
            ParameterProcessor::setParameterValue(0, controller, time);
            // send current parameter values to the MIDI output:
            for (int i = 1; i < getNrOfParameters(); i++) {
                int controllerNr = controller + i - 1;
                if (controllerNr <= 127) {
                    const ParameterProcessor::Parameter &parameter = getParameter(i);
                    // parameters with equal min and max values are not controllable via MIDI:
                    if (parameter.max != parameter.min) {
                        int controllerValue = qRound((parameter.value - parameter.min) * 127.0 / (parameter.max - parameter.min));
                        writeController(0, channel, controllerNr, controllerValue, time);
                    }
                }
            }
        } else {
            // change parameters based on incoming controller values:
            int parameterId = (int)controller - firstMidiController + 1;
            if ((parameterId > 0) && (parameterId < getNrOfParameters())) {
                // update the corresponding parameter value:
                const ParameterProcessor::Parameter &parameter = getParameter(parameterId);
                if (parameter.max != parameter.min) {
                    ParameterProcessor::setParameterValue(parameterId, (double)value * (parameter.max - parameter.min) / 127.0 + parameter.min, time);
                }
            }
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
                    writeController(0, channel, controller, controllerValue, time);
                }
            }
        }
        return true;
    } else {
        return false;
    }
}
