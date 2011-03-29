#include "jackparameterclient.h"

JackParameterClient::JackParameterClient(const QString &clientName, AudioProcessor *audioProcessor, MidiProcessor *midiProcessor, unsigned int channelMask, size_t ringBufferSize) :
    MidiProcessorClient(clientName, audioProcessor, midiProcessor, channelMask),
    ringBufferFromProcessToGui(ringBufferSize),
    ringBufferFromGuiToProcess(ringBufferSize),
    thread(new JackParameterThread(this, &ringBufferFromProcessToGui, &ringBufferFromGuiToProcess))
{
    QObject::connect(thread, SIGNAL(changedIntParameter(int,int)), this, SLOT(onChangedIntParameter(int,int)));
    QObject::connect(thread, SIGNAL(changedDoubleParameter(int,double)), this, SLOT(onChangedDoubleParameter(int,double)));
}

void JackParameterClient::saveState(QDataStream &stream)
{
    stream << parameters.size();
    for (int i = 0; i < parameters.size(); i++) {
        stream << parameters[i].type;
        if (parameters[i].type == TYPE_INT) {
            stream << parameters[i].intValue;
        } else {
            stream << parameters[i].doubleValue;
        }
    }
}

void JackParameterClient::loadState(QDataStream &stream)
{
    int nrOfParameters;
    stream >> nrOfParameters;
    if (nrOfParameters > parameters.size()) {
        parameters.resize(nrOfParameters);
    }
    for (int i = 0; i < nrOfParameters; i++) {
        stream >> parameters[i].type;
        if (parameters[i].type == TYPE_INT) {
            stream >> parameters[i].intValue;
        } else {
            stream >> parameters[i].doubleValue;
        }
    }
    // synchronize the changes to the process thread:
    ringBufferFromGuiToProcess.write(parameters.data(), parameters.size());
}

int JackParameterClient::getIntParameter(int parameterId)
{
    Q_ASSERT(parameterId < parameters.size());
    Q_ASSERT(parameters[parameterId].type == TYPE_INT);
    return parameters[parameterId].intValue;
}

double JackParameterClient::getDoubleParameter(int parameterId)
{
    Q_ASSERT(parameterId < parameters.size());
    Q_ASSERT(parameters[parameterId].type == TYPE_DOUBLE);
    return parameters[parameterId].doubleValue;
}

void JackParameterClient::changeIntParameter(int parameterId, int value)
{
    Q_ASSERT(parameterId < parameters.size());
    Q_ASSERT(parameters[parameterId].type == TYPE_INT);
    parameters[parameterId].intValue = value;
    parameters[parameterId].time = getEstimatedCurrentTime();
    // synchronize the change with the process thread:
    ringBufferFromGuiToProcess.write(parameters[parameterId]);
}

void JackParameterClient::changeDoubleParameter(int parameterId, double value)
{
    Q_ASSERT(parameterId < parameters.size());
    Q_ASSERT(parameters[parameterId].type == TYPE_DOUBLE);
    parameters[parameterId].doubleValue = value;
    parameters[parameterId].time = getEstimatedCurrentTime();
    // synchronize the change with the process thread:
    ringBufferFromGuiToProcess.write(parameters[parameterId]);
}

int JackParameterClient::registerParameter(ParameterType type)
{
    Parameter parameter;
    parameter.id = parameters.size();
    parameter.type = type;
    if (type == TYPE_INT) {
        parameter.intValue = 0;
    } else {
        parameter.doubleValue = 0;
    }
    parameter.time = 0;
    parameters.append(parameter);
    // synchronize the change with the process thread:
    ringBufferFromGuiToProcess.write(parameter);
    // return the new parameter id:
    return parameter.id;
}

void JackParameterClient::processChangeIntParameter(int parameterId, int value)
{
    Q_ASSERT(parameterId < parametersProcess.size());
    Q_ASSERT(parametersProcess[parameterId].type == TYPE_INT);
    parametersProcess[parameterId].intValue = value;
    // synchronization with the GUI thread is postponed until the end of the process cycle...
}

void JackParameterClient::processChangeDoubleParameter(int parameterId, double value)
{
    Q_ASSERT(parameterId < parametersProcess.size());
    Q_ASSERT(parametersProcess[parameterId].type == TYPE_DOUBLE);
    parametersProcess[parameterId].doubleValue = value;
    // synchronization with the GUI thread is postponed until the end of the process cycle...
}

int JackParameterClient::processGetIntParameter(int parameterId)
{
    Q_ASSERT(parameterId < parametersProcess.size());
    Q_ASSERT(parametersProcess[parameterId].type == TYPE_INT);
    return parametersProcess[parameterId].intValue;
}

double JackParameterClient::processGetDoubleParameter(double parameterId)
{
    Q_ASSERT(parameterId < parametersProcess.size());
    Q_ASSERT(parametersProcess[parameterId].type == TYPE_DOUBLE);
    return parametersProcess[parameterId].doubleValue;
}

bool JackParameterClient::init()
{
    thread->start();
    return MidiProcessorClient::init();
}

void JackParameterClient::deinit()
{
    thread->stop();
    MidiProcessorClient::deinit();
}

bool JackParameterClient::process(jack_nframes_t nframes)
{
    jack_nframes_t lastFrameTime = getLastFrameTime();
    // get audio port buffers:
    getPortBuffers(nframes);
    // get midi port buffer:
    getMidiPortBuffer(nframes);
    for (jack_nframes_t currentFrame = 0; currentFrame < nframes; ) {
        // get the next parameter change from the ring buffer, if there is any:
        if (ringBufferFromGuiToProcess.readSpace()) {
            Parameter parameter = ringBufferFromGuiToProcess.peek();
            // adjust time relative to the beginning of this frame:
            if (parameter.time + nframes < lastFrameTime) {
                // if time is too early, this is in the buffer for too long, adjust time accordingly:
                parameter.time = 0;
            } else {
                parameter.time = parameter.time + nframes - lastFrameTime;
            }
            if (parameter.time < nframes) {
                // process everything up to the event's time stamp:
                processMidi(currentFrame, parameter.time);
                currentFrame = parameter.time;
                // process the parameter change:
                ringBufferFromGuiToProcess.readAdvance(1);
                Q_ASSERT(parameter.id <= parametersProcess.size());
                if (parameter.id == parametersProcess.size()) {
                    // a new parameter has been registered:
                    parametersProcess.append(parameter);
                } else {
                    // an existing parameter has been changed:
                    parametersProcess[parameter.id] = parameter;
                }
            } else {
                processMidi(currentFrame, nframes);
                currentFrame = nframes;
            }
        } else {
            processMidi(currentFrame, nframes);
            currentFrame = nframes;
        }
    }
    // synchronize all changes that have been done during this process cycle to the GUI thread:
    synchronizeChangedParametersWithGui();
    return true;
}

void JackParameterClient::onChangedIntParameter(int parameterId, int value)
{
    // a parameter has been changed from the process thread, change it in the GUI thread:
    Q_ASSERT(parameterId < parameters.size());
    Q_ASSERT(parameters[parameterId].type == TYPE_INT);
    // trigger the corresponding signal only if the value has really changed:
    if (parameters[parameterId].intValue != value) {
        parameters[parameterId].intValue = value;
        changedIntParameter(parameterId, value);
    }
}

void JackParameterClient::onChangedDoubleParameter(int parameterId, double value)
{
    // a parameter has been changed from the process thread, change it in the GUI thread:
    Q_ASSERT(parameterId < parameters.size());
    Q_ASSERT(parameters[parameterId].type == TYPE_DOUBLE);
    // trigger the corresponding signal only if the value has reallychanged:
    if (parameters[parameterId].doubleValue != value) {
        parameters[parameterId].doubleValue = value;
        changedIntParameter(parameterId, value);
    }
}

void JackParameterClient::synchronizeChangedParametersWithGui()
{
    /*
      Always synchronize all parameters, otherwise process and GUI thread
      could become out of sync through parameter changes which are sent both ways
      approximately at the same time.
      */
    ringBufferFromProcessToGui.write(parametersProcess.data(), parametersProcess.size());
    // wake the associated thread:
    thread->wake();
}

JackParameterThread::JackParameterThread(JackParameterClient *client, JackRingBuffer<JackParameterClient::Parameter> *ringBufferFromProcessToGui_, JackRingBuffer<JackParameterClient::Parameter> *ringBufferFromGuiToProcess_) :
    JackThread(client, 0),
    ringBufferFromProcessToGui(ringBufferFromProcessToGui_),
    ringBufferFromGuiToProcess(ringBufferFromGuiToProcess_)
{}

void JackParameterThread::processDeferred()
{
    // read from the ring buffer and trigger the corresponding signals:
    for (; ringBufferFromProcessToGui->readSpace(); ) {
        JackParameterClient::Parameter parameter = ringBufferFromProcessToGui->read();
        if (parameter.type == JackParameterClient::TYPE_INT) {
            changedIntParameter(parameter.id, parameter.intValue);
        } else if (parameter.type == JackParameterClient::TYPE_INT) {
            changedIntParameter(parameter.id, parameter.doubleValue);
        }
    }
}
