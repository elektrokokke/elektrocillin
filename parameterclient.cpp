/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "parameterclient.h"
#include "graphicscontinuouscontrolitem.h"
#include <QPen>
#include <QBrush>

ParameterClient::ParameterClient(const QString &clientName, AudioProcessor *audioProcessor, MidiProcessor *midiProcessor, EventProcessor *eventProcessor, ParameterProcessor *processParameterProcessor_, ParameterProcessor *guiParameterProcessor_, size_t ringBufferSize) :
    EventProcessorClient(clientName, audioProcessor, midiProcessor, eventProcessor, ringBufferSize),
    processParameterProcessor(processParameterProcessor_),
    guiParameterProcessor(guiParameterProcessor_),
    ringBufferFromProcessToGui(ringBufferSize),
    ringBufferFromGuiToProcess(ringBufferSize),
    thread(new ParameterThread(this, &ringBufferFromProcessToGui, &ringBufferFromGuiToProcess))
{
    activateMidiInput(true);
    activateMidiOutput(true);
    if (midiProcessor) {
        midiProcessor->setMidiWriter(this);
    }
    QObject::connect(thread, SIGNAL(changedParameterValue(int,double,double,double)), this, SLOT(onChangedParameterValue(int,double,double,double)));
}

void ParameterClient::saveState(QDataStream &stream)
{
    EventProcessorClient::saveState(stream);
    for (int i = 0; i < guiParameterProcessor->getNrOfParameters(); i++) {
        const ParameterProcessor::Parameter &parameter = guiParameterProcessor->getParameter(i);
        stream << parameter.name;
        stream << parameter.value;
        stream << parameter.min;
        stream << parameter.max;
        stream << parameter.resolution;
    }
}

void ParameterClient::loadState(QDataStream &stream)
{
    EventProcessorClient::loadState(stream);
    for (int i = 0; i < guiParameterProcessor->getNrOfParameters(); i++) {
        ParameterProcessor::Parameter &parameter = guiParameterProcessor->getParameter(i);
        stream >> parameter.name;
        stream >> parameter.value;
        stream >> parameter.min;
        stream >> parameter.max;
        stream >> parameter.resolution;
    }
    *processParameterProcessor = *guiParameterProcessor;
}

int ParameterClient::getNrOfParameters() const
{
    return guiParameterProcessor->getNrOfParameters();
}

const ParameterProcessor::Parameter & ParameterClient::getParameter(int parameterId) const
{
    return guiParameterProcessor->getParameter(parameterId);
}

QGraphicsItem * ParameterClient::createGraphicsItem()
{
    return new ParameterGraphicsItem(this);
}

void ParameterClient::changeParameterValue(int index, double value)
{
    Q_ASSERT(index < guiParameterProcessor->getNrOfParameters());
    const ParameterProcessor::Parameter &parameter = guiParameterProcessor->getParameter(index);
    if (guiParameterProcessor->setParameterValue(index, value, parameter.min, parameter.max, 0)) {
        changedParameterValue(index, parameter.value, parameter.min, parameter.max);
        ParameterChange change;
        change.id = index;
        change.value = parameter.value;
        change.min = parameter.min;
        change.max = parameter.max;
        change.time = getEstimatedCurrentTime();
        // send the change to the process thread:
        ringBufferFromGuiToProcess.write(change);
    }
}

void ParameterClient::changeParameterValue(int index, double value, double min, double max)
{
    Q_ASSERT(index < guiParameterProcessor->getNrOfParameters());
    if (guiParameterProcessor->setParameterValue(index, value, min, max, 0)) {
        const ParameterProcessor::Parameter &parameter = guiParameterProcessor->getParameter(index);
        changedParameterValue(index, parameter.value, parameter.min, parameter.max);
        ParameterChange change;
        change.id = index;
        change.value = parameter.value;
        change.min = parameter.min;
        change.max = parameter.max;
        change.time = getEstimatedCurrentTime();
        // send the change to the process thread:
        ringBufferFromGuiToProcess.write(change);
    }
}

void ParameterClient::changeParameterBounds(int index, double min, double max)
{
    Q_ASSERT(index < guiParameterProcessor->getNrOfParameters());
    const ParameterProcessor::Parameter &parameter = guiParameterProcessor->getParameter(index);
    if (guiParameterProcessor->setParameterValue(index, parameter.value, min, max, 0)) {
        changedParameterValue(index, parameter.value, parameter.min, parameter.max);
        ParameterChange change;
        change.id = index;
        change.value = parameter.value;
        change.min = parameter.min;
        change.max = parameter.max;
        change.time = getEstimatedCurrentTime();
        // send the change to the process thread:
        ringBufferFromGuiToProcess.write(change);
    }
}

bool ParameterClient::init()
{
    thread->start();
    return EventProcessorClient::init();
}

void ParameterClient::deinit()
{
    thread->stop();
    EventProcessorClient::deinit();
}

bool ParameterClient::process(jack_nframes_t nframes)
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

bool ParameterClient::processParameters(jack_nframes_t start, jack_nframes_t end, jack_nframes_t nframes)
{
    Q_ASSERT(processParameterProcessor);
    jack_nframes_t lastFrameTime = getLastFrameTime();
    for (jack_nframes_t currentFrame = start; currentFrame < end; ) {
        // get the next parameter change from the ring buffer, if there is any:
        if (ringBufferFromGuiToProcess.readSpace()) {
            ParameterChange change = ringBufferFromGuiToProcess.peek();
            Q_ASSERT(change.id < processParameterProcessor->getNrOfParameters());
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
                processParameterProcessor->setParameterValue(change.id, change.value, change.min, change.max, change.time);
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

void ParameterClient::synchronizeChangedParametersWithGui()
{
    /*
      Always synchronize changed parameters, otherwise process and GUI thread
      could become out of sync through parameter changes which are sent both ways
      approximately at the same time.
      */
    for (int i = 0; i < processParameterProcessor->getNrOfParameters(); i++) {
        if (processParameterProcessor->hasParameterChanged(i)) {
            const ParameterProcessor::Parameter &parameter = processParameterProcessor->getParameter(i);
            ParameterChange change;
            change.id = i;
            change.value = parameter.value;
            change.min = parameter.min;
            change.max = parameter.max;
            change.time = 0;
            ringBufferFromProcessToGui.write(change);
        }
    }
    processParameterProcessor->resetParameterChanged();
    // wake the associated thread:
    thread->wake();
}

void ParameterClient::onChangedParameterValue(int index, double value, double min, double max)
{
    // a parameter has been changed from the process thread, change it in the GUI thread:
    if (guiParameterProcessor->setParameterValue(index, value, min, max, 0)) {
        changedParameterValue(index, value, min, max);
    }
}

ParameterThread::ParameterThread(ParameterClient *client, JackRingBuffer<ParameterClient::ParameterChange> *ringBufferFromProcessToGui_, JackRingBuffer<ParameterClient::ParameterChange> *ringBufferFromGuiToProcess_) :
    JackThread(client, 0),
    ringBufferFromProcessToGui(ringBufferFromProcessToGui_),
    ringBufferFromGuiToProcess(ringBufferFromGuiToProcess_)
{}

void ParameterThread::processDeferred()
{
    // read from the ring buffer and trigger the corresponding signals:
    for (; ringBufferFromProcessToGui->readSpace(); ) {
        ParameterClient::ParameterChange change = ringBufferFromProcessToGui->read();
        changedParameterValue(change.id, change.value, change.min, change.max);
    }
}

ParameterGraphicsItem::ParameterGraphicsItem(ParameterClient *client_, QGraphicsItem *parent) :
    QGraphicsRectItem(parent),
    client(client_)
{
    setFlags(QGraphicsItem::ItemIsFocusable);
    setPen(QPen(QBrush(Qt::black), 1));
    setBrush(QBrush(Qt::white));
    QObject::connect(client, SIGNAL(changedParameterValue(int,double,double,double)), this, SLOT(onClientChangedParameterValue(int,double,double,double)));
    int padding = 4;
    QRectF rectControls;
    qreal y = 0;
    // create controls for editing the client's parameters:
    for (int i = 0; i < client->getNrOfParameters(); i++) {
        const ParameterProcessor::Parameter &parameter = client->getParameter(i);
        if (parameter.min != parameter.max) {
            GraphicsContinuousControlItem *control = new GraphicsContinuousControlItem(parameter.name, parameter.min, parameter.max, parameter.value, qMin(800.0, qMax(200.0, qAbs(parameter.max - parameter.min) / qMax(parameter.resolution, 0.01))), GraphicsContinuousControlItem::HORIZONTAL, 'g', -1, parameter.resolution, this);
            QObject::connect(control, SIGNAL(valueChanged(double)), this, SLOT(onGuiChangedParameterValue(double)));
            control->setPos(QPointF(padding, padding + y));
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

void ParameterGraphicsItem::focusInEvent(QFocusEvent *)
{
    setZValue(1);
}

void ParameterGraphicsItem::focusOutEvent(QFocusEvent *)
{
    setZValue(0);
}

void ParameterGraphicsItem::onGuiChangedParameterValue(double value)
{
    // determine the parameter id and the corresponding control by the sender:
    int id = mapSenderToId.value(sender(), -1);
    Q_ASSERT(id != -1);
    // send the change to the client:
    client->changeParameterValue(id, value);
}

void ParameterGraphicsItem::onClientChangedParameterValue(int index, double value, double min, double max)
{
    Q_ASSERT(index < controls.size());
    if (controls[index]) {
        // reflect the new value in the appropriate control:
        controls[index]->setMinValue(min);
        controls[index]->setMaxValue(max);
        controls[index]->setValue(value, false);
    }
}
