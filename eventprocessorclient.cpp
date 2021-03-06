/*
    Copyright 2011 Arne Jacobs <jarne@jarne.de>

    This file is part of elektrocillin.

    Elektrocillin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Elektrocillin is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Elektrocillin.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "eventprocessorclient.h"

EventProcessorClient::EventProcessorClient(const QString &clientName, AudioProcessor *audioProcessor, MidiProcessor *midiProcessor, EventProcessor *eventProcessor_, size_t ringBufferSize) :
    MidiProcessorClient(clientName, audioProcessor, midiProcessor),
    eventProcessor(eventProcessor_),
    ringBuffer(ringBufferSize)
{}

EventProcessorClient::EventProcessorClient(const QString &clientName, const QStringList &audioInputPortNames, const QStringList &audioOutputPortNames, const QStringList &midiInputPortNames, const QStringList &midiOutputPortNames, size_t ringBufferSize) :
    MidiProcessorClient(clientName, audioInputPortNames, audioOutputPortNames, midiInputPortNames, midiOutputPortNames),
    eventProcessor(0),
    ringBuffer(ringBufferSize)
{}

EventProcessor * EventProcessorClient::getEventProcessor()
{
    return eventProcessor;
}

bool EventProcessorClient::postEvent(RingBufferEvent *event)
{
    if (isActive()) {
        jack_nframes_t time = getEstimatedCurrentTime();
        return ringBuffer.sendEvent(event, time);
    } else {
        return false;
    }
}

bool EventProcessorClient::postEvents(const QVector<RingBufferEvent*> &events)
{
    if (isActive()) {
        int writtenEvents = 0;
        jack_nframes_t time = getEstimatedCurrentTime();
        for (int i = 0; i < events.size(); i++) {
            if (ringBuffer.sendEvent(events[i], time)) {
                writtenEvents++;
            }
        }
        return (writtenEvents == events.size());
    } else {
        return false;
    }
}

bool EventProcessorClient::process(jack_nframes_t nframes)
{
    // get port buffers:
    getAudioPortBuffers(nframes);
    getMidiPortBuffers(nframes);
    // process all events:
    processEvents(0, nframes, nframes);
    return true;
}

bool EventProcessorClient::processEvents(jack_nframes_t start, jack_nframes_t end, jack_nframes_t nframes)
{
    jack_nframes_t lastFrameTime = getLastFrameTime();
    for (jack_nframes_t currentFrame = start; currentFrame < end; ) {
        // get the next event from the ring buffer, if there is any:
        if (ringBuffer.hasEvents()) {
            jack_nframes_t eventTime = ringBuffer.peekEventTime();
            // adjust time relative to the beginning of this frame:
            if (eventTime + nframes < lastFrameTime) {
                // if time is too early, this is in the buffer for too long, adjust time accordingly:
                eventTime = 0;
            } else {
                eventTime = eventTime + nframes - lastFrameTime;
            }
            if (eventTime < end) {
                // process everything up to the event's time stamp:
                processMidi(currentFrame, eventTime);
                currentFrame = eventTime;
                // process the event:
                RingBufferEvent *event = ringBuffer.readEvent();
                processEvent(event, eventTime);
                // have the event deleted in the creator thread:
                ringBuffer.returnEvent(event);
            } else {
                processMidi(currentFrame, end);
                currentFrame = end;
            }
        } else {
            processMidi(currentFrame, end);
            currentFrame = end;
        }
    }
    return true;
}

bool EventProcessorClient::processEvent(const RingBufferEvent *event, jack_nframes_t time)
{
    Q_ASSERT(eventProcessor);
    return getEventProcessor()->processEvent(event, time);
}
