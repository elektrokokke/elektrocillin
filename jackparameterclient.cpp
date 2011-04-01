#include "jackparameterclient.h"
#include "graphicscontinuouscontrolitem.h"

JackParameterClient::JackParameterClient(const QString &clientName, AudioProcessor *audioProcessor, MidiProcessor *midiProcessor, EventProcessor *eventProcessor, ParameterProcessor *parameterProcessor_, size_t ringBufferSize) :
    EventProcessorClient(clientName, audioProcessor, midiProcessor, eventProcessor, ringBufferSize),
    parameterProcessor(parameterProcessor_),
    ringBufferFromProcessToGui(ringBufferSize),
    ringBufferFromGuiToProcess(ringBufferSize),
    thread(new JackParameterThread(this, &ringBufferFromProcessToGui, &ringBufferFromGuiToProcess))
{
    QObject::connect(thread, SIGNAL(changedParameterValue(int,double)), this, SLOT(onChangedParameterValue(int,double)));
    QObject::connect(thread, SIGNAL(changedParameterValue(int,double)), this, SIGNAL(changedParameterValue(int,double)));
    QObject::connect(thread, SIGNAL(changedParameters()), this, SIGNAL(changedParameters()));
    for (int i = 0; i < parameterProcessor->getNrOfParameters(); i++) {
        parameters.append(parameterProcessor->getParameter(i));
    }
}

void JackParameterClient::saveState(QDataStream &stream)
{
    for (int i = 0; i < parameters.size(); i++) {
        stream << parameters[i].name;
        stream << parameters[i].value;
        stream << parameters[i].min;
        stream << parameters[i].max;
        stream << parameters[i].resolution;
    }
}

void JackParameterClient::loadState(QDataStream &stream)
{
    for (int i = 0; i < parameters.size(); i++) {
        stream >> parameters[i].name;
        stream >> parameters[i].value;
        stream >> parameters[i].min;
        stream >> parameters[i].max;
        stream >> parameters[i].resolution;
        parameterProcessor->setParameterValue(i, parameters[i].value);
    }
}

int JackParameterClient::getNrOfParameters() const
{
    return parameters.size();
}

const ParameterProcessor::Parameter & JackParameterClient::getParameter(int parameterId) const
{
    Q_ASSERT(parameterId < parameters.size());
    return parameters[parameterId];
}

double JackParameterClient::getParameterValue(int parameterId)
{
    Q_ASSERT(parameterId < parameters.size());
    return parameters[parameterId].value;
}

QGraphicsItem * JackParameterClient::createGraphicsItem()
{
    return new JackParameterGraphicsItem(this);
}

void JackParameterClient::changeParameterValue(int parameterId, double value)
{
    Q_ASSERT(parameterId < parameters.size());
    ParameterChange change;
    change.id = parameterId;
    change.value = value;
    change.time = getEstimatedCurrentTime();
    // send the change with the process thread:
    ringBufferFromGuiToProcess.write(change);
}

bool JackParameterClient::init()
{
    thread->start();
    return EventProcessorClient::init();
}

void JackParameterClient::deinit()
{
    thread->stop();
    EventProcessorClient::deinit();
}

bool JackParameterClient::process(jack_nframes_t nframes)
{
    // get port buffers:
    getPortBuffers(nframes);
    getMidiPortBuffer(nframes);
    // process all parameter changes:
    processParameters(0, nframes, nframes);
    // synchronize all changes that have been done during this process cycle to the GUI thread:
    synchronizeChangedParametersWithGui();
    return true;
}

bool JackParameterClient::processParameters(jack_nframes_t start, jack_nframes_t end, jack_nframes_t nframes)
{
    Q_ASSERT(parameterProcessor);
    jack_nframes_t lastFrameTime = getLastFrameTime();
    for (jack_nframes_t currentFrame = start; currentFrame < end; ) {
        // get the next parameter change from the ring buffer, if there is any:
        if (ringBufferFromGuiToProcess.readSpace()) {
            ParameterChange change = ringBufferFromGuiToProcess.peek();
            Q_ASSERT(change.id < parameterProcessor->getNrOfParameters());
            // adjust time relative to the beginning of this frame:
            if (change.time + nframes < lastFrameTime) {
                // if time is too early, this is in the buffer for too long, adjust time accordingly:
                change.time = 0;
            } else {
                change.time = change.time + nframes - lastFrameTime;
            }
            if (change.time < end) {
                // process everything up to the event's time stamp:
                processEvents(currentFrame, change.time, nframes);
                currentFrame = change.time;
                // process the parameter change:
                ringBufferFromGuiToProcess.readAdvance(1);
                parameterProcessor->setParameterValue(change.id, change.value);
            } else {
                processEvents(currentFrame, end, nframes);
                currentFrame = end;
            }
        } else {
            processEvents(currentFrame, end, nframes);
            currentFrame = end;
        }
    }
    return true;
}

void JackParameterClient::synchronizeChangedParametersWithGui()
{
    /*
      Always synchronize changed parameters, otherwise process and GUI thread
      could become out of sync through parameter changes which are sent both ways
      approximately at the same time.
      */
    for (int i = 0; i < parameterProcessor->getNrOfParameters(); i++) {
        if (parameterProcessor->hasParameterChanged(i)) {
            ParameterChange change;
            change.id = i;
            change.value = parameterProcessor->getParameter(i).value;
            change.time = 0;
            ringBufferFromProcessToGui.write(change);
        }
    }
    // wake the associated thread:
    thread->wake();
}

void JackParameterClient::onChangedParameterValue(int parameterId, double value)
{
    // a parameter has been changed from the process thread, change it in the GUI thread:
    Q_ASSERT(parameterId < parameters.size());
    parameters[parameterId].value = value;
}

JackParameterThread::JackParameterThread(JackParameterClient *client, JackRingBuffer<JackParameterClient::ParameterChange> *ringBufferFromProcessToGui_, JackRingBuffer<JackParameterClient::ParameterChange> *ringBufferFromGuiToProcess_) :
    JackThread(client, 0),
    ringBufferFromProcessToGui(ringBufferFromProcessToGui_),
    ringBufferFromGuiToProcess(ringBufferFromGuiToProcess_)
{}

void JackParameterThread::processDeferred()
{
    // read from the ring buffer and trigger the corresponding signals:
    bool changes = ringBufferFromProcessToGui->readSpace();
    for (; ringBufferFromProcessToGui->readSpace(); ) {
        JackParameterClient::ParameterChange change = ringBufferFromProcessToGui->read();
        changedParameterValue(change.id, change.value);
    }
    if (changes) {
        changedParameters();
    }
}

JackParameterGraphicsItem::JackParameterGraphicsItem(JackParameterClient *client_, QGraphicsItem *parent) :
    QGraphicsRectItem(parent),
    client(client_)
{
    setFlags(QGraphicsItem::ItemIsFocusable);
    QObject::connect(client, SIGNAL(changedParameterValue(int,double)), this, SLOT(onClientChangedParameterValue(int,double)));
    int padding = 4;
    QRectF rectControls;
    qreal y = 0;
    // create controls for editing the client's parameters:
    for (int i = 0; i < client->getNrOfParameters(); i++) {
        const ParameterProcessor::Parameter &parameter = client->getParameter(i);
        if (parameter.min != parameter.max) {
            GraphicsContinuousControlItem *control = new GraphicsContinuousControlItem(parameter.name, parameter.min, parameter.max, parameter.value, qMin(800.0, qMax(200.0, qAbs(parameter.max - parameter.min) / qMax(parameter.resolution, 0.01))), GraphicsContinuousControlItem::HORIZONTAL, 'g', -1, parameter.resolution, this);
            QObject::connect(control, SIGNAL(valueChanged(double)), this, SLOT(onGuiChangedParameterValue(double)));
            control->setPos(QPointF(0, y));
            y += control->rect().height() + padding;
            rectControls |= control->rect().translated(control->pos());
            controls.append(control);
            mapSenderToId[control] = i;
        } else {
            controls.append(0);
        }
    }
    setRect(rectControls.adjusted(-padding, -padding, padding, padding));
}

void JackParameterGraphicsItem::focusInEvent(QFocusEvent * event)
{
    setZValue(1);
}

void JackParameterGraphicsItem::focusOutEvent(QFocusEvent * event)
{
    setZValue(0);
}

void JackParameterGraphicsItem::onGuiChangedParameterValue(double value)
{
    // determine the parameter id and the corresponding control by the sender:
    int id = mapSenderToId.value(sender(), -1);
    Q_ASSERT(id != -1);
    // send the change to the client:
    client->changeParameterValue(id, value);
}

void JackParameterGraphicsItem::onClientChangedParameterValue(int parameterId, double value)
{
    Q_ASSERT(parameterId < controls.size());
    if (controls[parameterId]) {
        // reflect the new value in the appropriate control:
        controls[parameterId]->setValue(value);
    }
}
