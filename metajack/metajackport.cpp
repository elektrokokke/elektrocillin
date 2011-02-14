#include "metajackport.h"
#include "metajackclient.h"
#include "metajackcontextnew.h"
#include <sstream>
#include <cassert>
#include <list>

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

MetaJackPortProcess::MetaJackPortProcess(jack_port_id_t id, const std::string &shortName, const std::string &type, int flags) :
    MetaJackPortBase(id, shortName, type, flags),
    bufferSize(0),
    buffer(0)
{}

void * MetaJackPortProcess::getBuffer(jack_nframes_t nframes)
{
    // resize the buffer if necessary:
    if (nframes * sizeof(jack_default_audio_sample_t) > bufferSize) {
        if (buffer) {
            delete [] buffer;
        }
        bufferSize = nframes * sizeof(jack_default_audio_sample_t);
        buffer = new char[bufferSize];
        // if this is a MIDI port, write its size to the head of the buffer:
        if (getType() == JACK_DEFAULT_MIDI_TYPE) {
            MetaJackContextNew::MetaJackContextMidiBufferHead *head = (MetaJackContextNew::MetaJackContextMidiBufferHead*)buffer;
            head->bufferSize = bufferSize - sizeof(MetaJackContextNew::MetaJackContextMidiBufferHead);
            head->midiDataSize = head->midiEventCount = head->lostMidiEvents = 0;
        }
    }
    return buffer;
}

bool MetaJackPortProcess::clearBuffer()
{
    if (getType() == JACK_DEFAULT_AUDIO_TYPE) {
        // clearing means setting everything to zero:
        memset(buffer, 0, bufferSize);
        return true;
    } else if (getType() == JACK_DEFAULT_MIDI_TYPE) {
        MetaJackContextNew::midi_clear_buffer(buffer);
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
        size_t nframes = bufferSize / sizeof(jack_default_audio_sample_t);
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
            for (jack_nframes_t j = 0; j < MetaJackContextNew::midi_get_event_count(midiOutputBuffer); j++) {
                jack_midi_event_t event;
                MetaJackContextNew::midi_event_get(&event, midiOutputBuffer, j);
                events.push_back(event);
            }
        }
        // now sort the events by time:
        events.sort(MetaJackContextNew::compare_midi_events);
        // write them to the input buffer:
        void *midiInputBuffer = buffer;
        for (std::list<jack_midi_event_t>::iterator i = events.begin(); i != events.end(); i++) {
            MetaJackContextNew::midi_event_write(midiInputBuffer, i->time, i->buffer, i->size);
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

MetaJackPortNew::MetaJackPortNew(MetaJackClientNew *client, jack_port_id_t id, const std::string &shortName, const std::string &type, int flags) :
    MetaJackPortBase(id, shortName, type, flags),
    twin(0)
{
    setClient(client);
}

void MetaJackPortNew::createProcessPort()
{
    if (!twin) {
        twin = new MetaJackPortProcess(getId(), getShortName(), getType(), getFlags());
    }
}

MetaJackPortProcess * MetaJackPortNew::getProcessPort()
{
    return twin;
}
