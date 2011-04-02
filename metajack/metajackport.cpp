/*
    Copyright 2011 Arne Jacobs

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

#include "metajackport.h"
#include "metajackclient.h"
#include "metajackcontext.h"
#include <sstream>
#include <cassert>
#include <list>
#include <memory.h>

MetaJackPortBase::MetaJackPortBase(jack_port_id_t id_, const std::string &shortName_, const std::string &type_, int flags_) :
    client(0),
    id(id_),
    shortName(shortName_),
    type(type_),
    flags(flags_)
{
}

MetaJackPortBase::~MetaJackPortBase()
{
    disconnect();
    if (client) {
        client->removePort(this);
    }
}

void MetaJackPortBase::setClient(MetaJackClientBase *client)
{
    this->client = client;
    std::stringstream fullNameStream;
    fullNameStream << client->getName() << ":" << shortName;
    fullName = fullNameStream.str();
    client->addPort(this);
}

const std::string & MetaJackPortBase::getShortName() const
{
    return shortName;
}

void MetaJackPortBase::setShortName(const std::string &shortName_)
{
    shortName = shortName_;
    std::stringstream fullNameStream;
    fullNameStream << client->getName() << ":" << shortName;
    fullName = fullNameStream.str();
}

const std::string & MetaJackPortBase::getFullName() const
{
    return fullName;
}

const std::string & MetaJackPortBase::getType() const
{
    return type;
}

jack_port_id_t MetaJackPortBase::getId() const
{
    return id;
}

int MetaJackPortBase::getFlags() const
{
    return flags;
}

bool MetaJackPortBase::isInput() const
{
    return (flags & JackPortIsInput);
}

size_t MetaJackPortBase::getConnectionCount() const
{
    return connectedPorts.size();
}

bool MetaJackPortBase::isConnectedTo(MetaJackPortBase *port) const
{
    return (connectedPorts.find(port) != connectedPorts.end());
}

bool MetaJackPortBase::isIndirectInputOf(MetaJackPortBase *port) const
{
    if (isInput()) {
        return client->isIndirectInputOf(port);
    } else {
        for (std::set<MetaJackPortBase*>::const_iterator i = connectedPorts.begin(); i != connectedPorts.end(); i++) {
            MetaJackPortBase *connectedPort = *i;
            if (connectedPort->isIndirectInputOf(port)) {
                return true;
            }
        }
        return false;
    }
}

const char ** MetaJackPortBase::getConnections() const
{
    if (getConnectionCount()) {
        char ** names = new char*[getConnectionCount() + 1];
        size_t index = 0;
        for (std::set<MetaJackPortBase*>::const_iterator i = connectedPorts.begin(); i != connectedPorts.end(); i++, index++) {
            MetaJackPortBase *port = *i;
            names[index] = new char[port->getFullName().length() + 1];
            memcpy(names[index], port->getFullName().c_str(), port->getFullName().length() + 1);
        }
        names[index] = 0;
        return (const char**)names;
    } else {
        return 0;
    }
}

void MetaJackPortBase::disconnect()
{
    for (; connectedPorts.size(); ) {
        MetaJackPortBase *port = *connectedPorts.begin();
        disconnect(port);
    }
}

void MetaJackPortBase::disconnect(MetaJackPortBase *port)
{
    connectedPorts.erase(port);
    port->connectedPorts.erase(this);
}

void MetaJackPortBase::connect(MetaJackPortBase *port)
{
    connectedPorts.insert(port);
    port->connectedPorts.insert(this);
}

bool MetaJackPortBase::belongsTo(const MetaJackClientBase *client_) const
{
    return (client == client_);
}

MetaJackClientBase * MetaJackPortBase::getClient()
{
    return client;
}

const std::set<MetaJackPortBase*> & MetaJackPortBase::getConnectedPorts()
{
    return connectedPorts;
}

MetaJackPortProcess::MetaJackPortProcess(jack_port_id_t id, const std::string &shortName, const std::string &type, int flags, jack_nframes_t bufferSize) :
    MetaJackPortBase(id, shortName, type, flags),
    bufferSizeInBytes(0),
    buffer(0)
{
    changeBufferSize(bufferSize);
}

MetaJackPortProcess::~MetaJackPortProcess()
{
    delete [] buffer;
}

void * MetaJackPortProcess::getBuffer(jack_nframes_t nframes)
{
    // this will be called from the process thread, so no memory allocation must be done here!
    assert(nframes * sizeof(jack_default_audio_sample_t) <= bufferSizeInBytes);
    return buffer;
}

void MetaJackPortProcess::changeBufferSize(jack_nframes_t bufferSize)
{
    if (bufferSizeInBytes != bufferSize * sizeof(jack_default_audio_sample_t)) {
        if (buffer) delete [] buffer;
        bufferSizeInBytes = bufferSize * sizeof(jack_default_audio_sample_t);
        buffer = new char [bufferSizeInBytes];
        // if this is a MIDI port, write its size to the head of the buffer:
        if (getType() == JACK_DEFAULT_MIDI_TYPE) {
            MetaJackContext::midi_init_buffer(buffer, bufferSizeInBytes);
        }
    }
}

bool MetaJackPortProcess::clearBuffer()
{
    if (getType() == JACK_DEFAULT_AUDIO_TYPE) {
        // clearing means setting everything to zero:
        memset(buffer, 0, bufferSizeInBytes);
        return true;
    } else if (getType() == JACK_DEFAULT_MIDI_TYPE) {
        MetaJackContext::midi_clear_buffer(buffer);
        return true;
    } else {
        return false;
    }
}

bool MetaJackPortProcess::mergeConnectedBuffers()
{
    assert(isInput());
    // first clear the buffer:
    if (!clearBuffer()) {
        return false;
    }
    if (getType() == JACK_DEFAULT_AUDIO_TYPE) {
        jack_default_audio_sample_t *destBuffer = (jack_default_audio_sample_t*)buffer;
        size_t nframes = bufferSizeInBytes / sizeof(jack_default_audio_sample_t);
        // add audio from all connected output buffers:
        for (std::set<MetaJackPortBase*>::iterator i = connectedPorts.begin(); i != connectedPorts.end(); i++) {
            MetaJackPortProcess *connectedPort = (MetaJackPortProcess*)*i;
            jack_default_audio_sample_t *sourceBuffer = (jack_default_audio_sample_t*)connectedPort->buffer;
            for (jack_nframes_t j = 0; j < nframes; j++) {
                destBuffer[j] += sourceBuffer[j];
            }
        }
        return true;
    } else if (getType() == JACK_DEFAULT_MIDI_TYPE) {
        // first collect all midi events from connected output buffers:
        std::list<jack_midi_event_t> events;
        for (std::set<MetaJackPortBase*>::iterator i = connectedPorts.begin(); i != connectedPorts.end(); i++) {
            MetaJackPortProcess *connectedPort = (MetaJackPortProcess*)*i;
            void *midiOutputBuffer = connectedPort->buffer;
            for (jack_nframes_t j = 0; j < MetaJackContext::midi_get_event_count(midiOutputBuffer); j++) {
                jack_midi_event_t event;
                MetaJackContext::midi_event_get(&event, midiOutputBuffer, j);
                events.push_back(event);
            }
        }
        // now sort the events by time:
        events.sort(MetaJackContext::compare_midi_events);
        // write them to the input buffer:
        void *midiInputBuffer = buffer;
        for (std::list<jack_midi_event_t>::iterator i = events.begin(); i != events.end(); i++) {
            MetaJackContext::midi_event_write(midiInputBuffer, i->time, i->buffer, i->size);
        }
        return true;
    } else {
        return false;
    }
}

bool MetaJackPortProcess::process(std::set<MetaJackClientProcess*> &unprocessedClients, jack_nframes_t nframes)
{
    assert(isInput());
    // process all other clients that are connected to this input:
    for (std::set<MetaJackPortBase*>::iterator j = connectedPorts.begin(); j != connectedPorts.end(); j++) {
        MetaJackPortProcess *connectedPort = (MetaJackPortProcess*)*j;
        MetaJackClientProcess *client = (MetaJackClientProcess*)connectedPort->getClient();
        if (unprocessedClients.find(client) != unprocessedClients.end()) {
            if (!client->process(unprocessedClients, nframes)) {
                return false;
            }
        }
    }
    // merge all buffers connected to this input port:
    return mergeConnectedBuffers();
}

MetaJackPort::MetaJackPort(MetaJackClient *client, jack_port_id_t id, const std::string &shortName, const std::string &type, int flags) :
    MetaJackPortBase(id, shortName, type, flags),
    twin(0)
{
    setClient(client);
}

void MetaJackPort::createProcessPort(jack_nframes_t bufferSize)
{
    if (!twin) {
        twin = new MetaJackPortProcess(getId(), getShortName(), getType(), getFlags(), bufferSize);
    }
}

MetaJackPortProcess * MetaJackPort::getProcessPort()
{
    return twin;
}
