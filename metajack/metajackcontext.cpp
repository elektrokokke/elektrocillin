#include "metajackcontext.h"
#include <sstream>
#include <list>
#include <cassert>
#include <memory.h>
#include <QRegExp>
#include <QDebug>

MetaJackClient::MetaJackClient(MetaJackContext *context_, const std::string &name_, bool processThread_) :
    context(context_),
    name(name_),
    processCallback(0),
    processCallbackArgument(0),
    twin(0),
    processThread(processThread_)
{
    if (!residesInProcessThread()) {
        context->registeredClient(this);
    }
}

MetaJackClient::~MetaJackClient()
{
    if (!residesInProcessThread()) {
        // deactivate the client:
        deactivate();
        context->unregisteredClient(this);
    }
    // delete all ports (this will also remove all their connections):
    for (; ports.size(); ) {
        MetaJackPort *port = *ports.begin();
        delete port;
    }
}

const std::string & MetaJackClient::getName() const
{
    return name;
}

bool MetaJackClient::activate()
{
    if (residesInProcessThread() || isActive()) {
        return false;
    }
    getContext()->activatedClient(this);
    return true;
}

bool MetaJackClient::deactivate()
{
    if (residesInProcessThread() || !isActive()) {
        return false;
    }
    // disconnect all ports:
    disconnect();
    getContext()->deactivatedClient(this);
    return true;
}

bool MetaJackClient::setProcessCallback(JackProcessCallback processCallback, void *processCallbackArgument)
{
    if (residesInProcessThread() || isActive()) {
        return false;
    }
    this->processCallback = processCallback;
    this->processCallbackArgument = processCallbackArgument;
    return true;
}

MetaJackPort * MetaJackClient::registerPort(const std::string &port_name, const std::string &port_type, unsigned long flags, unsigned long buffer_size)
{
    // check if the given port name is not too long:
    if (getName().length() + port_name.length() + 1 > MetaJackPort::getNameSize()) {
        return 0;
    }
    // check if the given port name is already taken:
    std::stringstream stream;
    stream << getName() << ":" << port_name;
    std::string full_name = stream.str();
    if (getContext()->getPortByName(full_name)) {
        return 0;
    }
    // consider only audio and midi ports (to be able to ignore buffer_size for now):
    if ((port_type != JACK_DEFAULT_AUDIO_TYPE) && (port_type != JACK_DEFAULT_MIDI_TYPE)) {
        return 0;
    }
    // make sure that the port is either input or output:
    if (!(flags & JackPortIsInput) && !(flags & JackPortIsOutput)) {
        return 0;
    }
    // create a new unique id for this port:
    jack_port_id_t id = getContext()->createUniquePortId();
    // create a new port:
    MetaJackPort * port = new MetaJackPort(this, id, port_name, port_type, flags);
    return port;
}

bool MetaJackClient::hasPort(const MetaJackPort *port) const
{
    return (ports.find(const_cast<MetaJackPort*>(port)) != ports.end());
}

MetaJackContext * MetaJackClient::getContext()
{
    return context;
}

const MetaJackContext * MetaJackClient::getContext() const
{
    return context;
}

bool MetaJackClient::isActive() const
{
    return twin;
}

MetaJackClient * MetaJackClient::getTwin()
{
    if (twin) {
        return twin;
    }
    twin = new MetaJackClient(context, name, true);
    twin->processCallback = processCallback;
    twin->processCallbackArgument = processCallbackArgument;
    // duplicate the ports:
    for (std::set<MetaJackPort*>::iterator i = ports.begin(); i != ports.end(); i++) {
        MetaJackPort *port = *i;
        port->getTwin();
    }
    return twin;
}

void MetaJackClient::deleteTwin()
{
    if (twin) {
        delete twin;
        twin = 0;
    }
}

void MetaJackClient::addPort(MetaJackPort *port)
{
    ports.insert(port);
}

void MetaJackClient::removePort(MetaJackPort *port)
{
    ports.erase(port);
}

bool MetaJackClient::residesInProcessThread() const
{
    return processThread;
}

bool MetaJackClient::process(std::set<MetaJackClient*> &unprocessedClients, jack_nframes_t nframes)
{
    assert(residesInProcessThread());
    // recursively process all clients that are connected to this client's input ports first:
    for (std::set<MetaJackPort*>::iterator i = ports.begin(); i != ports.end(); i++) {
        MetaJackPort *port = *i;
        // make sure all the client's buffers are initialized:
        port->getBuffer(nframes);
        // process all other clients that are connected to one of our inputs first:
        if (port->isInput()) {
            if (!port->process(unprocessedClients, nframes)) {
                return false;
            }
        }
    }
    // process this client:
    if (processCallback) {
        int errorCode = processCallback(nframes, processCallbackArgument);
        if (errorCode) {
            return false;
        }
    }
    // processing succeeded:
    unprocessedClients.erase(this);
    return true;
}

void MetaJackClient::disconnect()
{
    // disconnect all ports:
    for (std::set<MetaJackPort*>::iterator i = ports.begin(); i != ports.end(); i++) {
        MetaJackPort *port = *i;
        port->disconnect();
    }
}

MetaJackPort::MetaJackPort(MetaJackClient *client_, jack_port_id_t id_, const std::string &short_name_, const std::string &type_, int flags_) :
    client(client_),
    id(id_),
    short_name(short_name_),
    type(type_),
    flags(flags_),
    bufferSize(0),
    buffer(0),
    twin(0)
{
    std::stringstream stream;
    stream << client->getName() << ":" << short_name;
    full_name = stream.str();
    client->addPort(this);
    if (!residesInProcessThread()) {
        getContext()->registeredPort(this);
    }
}

MetaJackPort::~MetaJackPort()
{
    // disconnect the port:
    disconnect();
    client->removePort(this);
    if (!residesInProcessThread()) {
        getContext()->unregisteredPort(this);
    }
    // delete the buffer:
    if (buffer) {
        delete [] buffer;
    }
}

const std::string & MetaJackPort::getShortName() const
{
    return short_name;
}

bool MetaJackPort::setShortName(const std::string &name)
{
    // make sure that the long port name is not longer than possible:
    std::string newName = name;
    size_t maxSize = getNameSize();
    if (newName.length() + client->getName().length() + 1 > maxSize) {
        // truncate the name:
        newName.resize(maxSize - client->getName().length() - 1);
    }
    // check if the port name is already taken:
    std::stringstream stream;
    stream << client->getName() << ":" << newName;
    std::string newFullName = stream.str();
    if (getContext()->getPortByName(newFullName)) {
        return false;
    }
    std::string oldFullName = getFullName();
    short_name = newName;
    full_name = newFullName;
    getContext()->renamedPort(oldFullName, newFullName);
    return true;
}

const std::string & MetaJackPort::getFullName() const
{
    return full_name;
}

const std::string & MetaJackPort::getType() const
{
    return type;
}

jack_port_id_t MetaJackPort::getId() const
{
    return id;
}

int MetaJackPort::getFlags() const
{
    return flags;
}

size_t MetaJackPort::getConnectionCount() const
{
    return connectedPorts.size();
}

bool MetaJackPort::isConnectedTo(MetaJackPort *port) const
{
    return connectedPorts.find(port) != connectedPorts.end();
}

bool MetaJackPort::isConnectedTo(const std::string &port_name) const
{
    MetaJackPort *port = getContext()->getPortByName(port_name);
    return isConnectedTo(port);
}

const char ** MetaJackPort::getConnections() const
{
    if (getConnectionCount()) {
        char ** names = new char*[getConnectionCount() + 1];
        size_t index = 0;
        for (std::set<MetaJackPort*>::iterator i = connectedPorts.begin(); i != connectedPorts.end(); i++, index++) {
            MetaJackPort *port = *i;
            names[index] = new char[port->getFullName().length() + 1];
            memcpy(names[index], port->getFullName().c_str(), port->getFullName().length() + 1);
        }
        names[index] = 0;
        return (const char**)names;
    } else {
        return 0;
    }
}

void * MetaJackPort::getBuffer(jack_nframes_t nframes)
{
    if (residesInProcessThread()) {
        // resize the buffer if necessary:
        if (nframes * sizeof(jack_default_audio_sample_t) > bufferSize) {
            if (buffer) {
                delete [] buffer;
            }
            bufferSize = nframes * sizeof(jack_default_audio_sample_t);
            buffer = new char[bufferSize];
            // if this is a MIDI port, write its size to the head of the buffer:
            if (type == JACK_DEFAULT_MIDI_TYPE) {
                MetaJackContextMidiBufferHead *head = (MetaJackContextMidiBufferHead*)buffer;
                head->bufferSize = bufferSize - sizeof(MetaJackContextMidiBufferHead);
                head->midiDataSize = head->midiEventCount = head->lostMidiEvents = 0;
            }
        }
        return buffer;
    } else {
        assert(twin);
        return twin->getBuffer(nframes);
    }
}

bool MetaJackPort::process(std::set<MetaJackClient*> &unprocessedClients, jack_nframes_t nframes)
{
    assert(residesInProcessThread());
    assert(isInput());
    // process all other clients that are connected to this input:
    for (std::set<MetaJackPort*>::iterator j = connectedPorts.begin(); j != connectedPorts.end(); j++) {
        MetaJackPort *connectedPort = *j;
        if (unprocessedClients.find(connectedPort->client) != unprocessedClients.end()) {
            if (!connectedPort->client->process(unprocessedClients, nframes)) {
                return false;
            }
        }
    }
    // merge all buffers connected to this input port:
    mergeConnectedBuffers();
    return true;
}

bool MetaJackPort::isActive() const
{
    return client->isActive();
}

void MetaJackPort::disconnect()
{
    for (std::set<MetaJackPort*>::iterator i = connectedPorts.begin(); i != connectedPorts.end(); i++) {
        MetaJackPort *connectedPort = *i;
        disconnect(connectedPort);
    }
}

void MetaJackPort::disconnect(MetaJackPort *port)
{
    connectedPorts.erase(port);
    port->connectedPorts.erase(this);
    if (!residesInProcessThread()) {
        getContext()->disconnectedPorts(this, port);
    }
}

void MetaJackPort::connect(MetaJackPort *port)
{
    // TODO: avoid circles in the graph...
    connectedPorts.insert(port);
    port->connectedPorts.insert(this);
    if (!residesInProcessThread()) {
        getContext()->connectedPorts(this, port);
    }
}

bool MetaJackPort::isInput()
{
    return (flags & JackPortIsInput);
}

size_t MetaJackPort::getNameSize()
{
    return jack_port_name_size();
}

size_t MetaJackPort::getTypeSize()
{
    return jack_port_type_size();
}

MetaJackContext * MetaJackPort::getContext()
{
    return client->getContext();
}

const MetaJackContext * MetaJackPort::getContext() const
{
    return client->getContext();
}

bool MetaJackPort::residesInProcessThread() const
{
    return client->residesInProcessThread();
}

MetaJackPort * MetaJackPort::getTwin()
{
    if (twin) {
        return twin;
    }
    // make sure the port's client has a twin (i.e. is active):
    assert(client->isActive());
    // create a twin port:
    twin = new MetaJackPort(client->getTwin(), id, short_name, type, flags);
    // add the twin port to the corresponding twin client:
    client->getTwin()->addPort(twin);
    // duplicate the connections:
    for (std::set<MetaJackPort*>::iterator i = connectedPorts.begin(); i != connectedPorts.end(); i++) {
        MetaJackPort *connectedPort = *i;
        twin->connectedPorts.insert(connectedPort->getTwin());
    }
    return twin;
}

void MetaJackPort::deleteTwin()
{
    if (twin) {
        delete twin;
        twin = 0;
    }
}

void MetaJackPort::clearBuffer()
{
    if (type == JACK_DEFAULT_AUDIO_TYPE) {
        // clearing means setting everything to zero:
        memset(buffer, 0, bufferSize);
    } else if (type == JACK_DEFAULT_MIDI_TYPE) {
        MetaJackContext::midi_clear_buffer(buffer);
    }
}

bool compare_midi_events(const jack_midi_event_t &event1, const jack_midi_event_t &event2) {
    return event1.time < event2.time;
}

void MetaJackPort::mergeConnectedBuffers()
{
    assert(isInput());
    // first clear the buffer:
    clearBuffer();
    if (type == JACK_DEFAULT_AUDIO_TYPE) {
        jack_default_audio_sample_t *destBuffer = (jack_default_audio_sample_t*)buffer;
        size_t nframes = bufferSize / sizeof(jack_default_audio_sample_t);
        // add audio from all connected output buffers:
        for (std::set<MetaJackPort*>::iterator i = connectedPorts.begin(); i != connectedPorts.end(); i++) {
            MetaJackPort *connectedPort = *i;
            jack_default_audio_sample_t *sourceBuffer = (jack_default_audio_sample_t*)connectedPort->buffer;
            for (jack_nframes_t j = 0; j < nframes; j++) {
                destBuffer[j] += sourceBuffer[j];
            }
        }
    } else if (type == JACK_DEFAULT_MIDI_TYPE) {
        // first collect all midi events from connected output buffers:
        std::list<jack_midi_event_t> events;
        for (std::set<MetaJackPort*>::iterator i = connectedPorts.begin(); i != connectedPorts.end(); i++) {
            MetaJackPort *connectedPort = *i;
            void *midiOutputBuffer = connectedPort->buffer;
            for (jack_nframes_t j = 0; j < MetaJackContext::midi_get_event_count(midiOutputBuffer); j++) {
                jack_midi_event_t event;
                MetaJackContext::midi_event_get(&event, midiOutputBuffer, j);
                events.push_back(event);
            }
        }
        // now sort the events by time:
        events.sort(compare_midi_events);
        // write them to the input buffer:
        void *midiInputBuffer = buffer;
        bool lost = false;
        for (std::list<jack_midi_event_t>::iterator i = events.begin(); !lost && (i != events.end()); i++) {
            lost = MetaJackContext::midi_event_write(midiInputBuffer, i->time, i->buffer, i->size);
        }
        // remember the number of lost midi events (those that could not be written to the midi buffer):
        MetaJackContextMidiBufferHead *head = (MetaJackContextMidiBufferHead*)midiInputBuffer;
        head->lostMidiEvents = events.size();
    }
}

//MetaJackContext * MetaJackContext::instance = &MetaJackContext::instance_;
//MetaJackContext MetaJackContext::instance_("meta_jack");

MetaJackContext::MetaJackContext(const std::string &name) :
    wrapperClient(0),
    wrapperAudioInputPort(0),
    wrapperAudioOutputPort(0),
    wrapperMidiInputPort(0),
    wrapperMidiOutputPort(0),
    uniquePortId(1),
    graphChangesRingBuffer(1024)
{
    // register at the real JACK server:
    wrapperClient = jack_client_open(name.c_str(), JackNullOption, 0);
    if (wrapperClient) {
        // register the process callback:
        jack_set_process_callback(wrapperClient, process, this);
        // register some ports:
        wrapperAudioInputPort = jack_port_register(wrapperClient, "Audio in", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
        wrapperAudioOutputPort = jack_port_register(wrapperClient, "Audio out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
        wrapperMidiInputPort = jack_port_register(wrapperClient, "Midi in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
        wrapperMidiOutputPort = jack_port_register(wrapperClient, "Midi out", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
        // activate the client:
        if (jack_activate(wrapperClient)) {
            jack_client_close(wrapperClient);
            wrapperClient = 0;
        }
    }
    if (wrapperClient) {
        MetaJackDummyInputClient *dummyInputClient = new MetaJackDummyInputClient(this, wrapperAudioInputPort, wrapperMidiInputPort);
        dummyInputClient->activate();
        MetaJackDummyOutputClient *dummyOutputClient = new MetaJackDummyOutputClient(this, wrapperAudioOutputPort, wrapperMidiOutputPort);
        dummyOutputClient->activate();
    }
}

MetaJackContext::~MetaJackContext()
{
    if (wrapperClient) {
        // first close all clients:
        for (; metaClients.size(); ) {
            MetaJackClient *client = *metaClients.begin();
            delete client;
        }
        // then shutdown the wrapper client:
        jack_client_close(wrapperClient);
    }
}

bool MetaJackContext::isActive() const
{
    return wrapperClient;
}

/**
  @param options currently only JackUseExactName is considered, all other options are ignored.
  */
MetaJackClient * MetaJackContext::openClient(const std::string &name, jack_options_t options)
{
    // meta clients can only be created if the wrapper client could be activated:
    if (!isActive()) {
        return 0;
    }
    // test if the name is not too long:
    if (name.length() > (size_t)getClientNameSize()) {
        return 0;
    }
    // test if the requested name is already taken:
    bool nameIsTaken = (metaClientsByName.find(name) != metaClientsByName.end());
    // if the name is taken but the JackUseExactName flag is given, we can't continue:
    if (nameIsTaken && (options & JackUseExactName)) {
        return 0;
    }
    std::string clientName = name;
    // if the name is taken, append a suffix:
    for (int suffix = 2; nameIsTaken; suffix++) {
        std::stringstream stream;
        stream << name << suffix;
        clientName = stream.str();
        nameIsTaken = (metaClientsByName.find(clientName) != metaClientsByName.end());
    }
    // create a new meta client (this will add the client to our lists and invoke the client registration callbacks):
    MetaJackClient *client = new MetaJackClient(this, clientName, false);
    return client;
}

int MetaJackContext::get_pid()
{
    assert(isActive());
    return jack_get_client_pid(jack_get_client_name(wrapperClient));
}

pthread_t MetaJackContext::get_thread_id()
{
    assert(isActive());
    return jack_client_thread_id(wrapperClient);
}

bool MetaJackContext::is_realtime()
{
    assert(isActive());
    return jack_is_realtime(wrapperClient);
}

int MetaJackContext::set_thread_init_callback (MetaJackClient *client, JackThreadInitCallback thread_init_callback, void *arg)
{
    if (client->isActive()) {
        return 1;
    }
    threadInitCallbacks[client] = std::make_pair(thread_init_callback, arg);
    return 0;
}

void MetaJackContext::set_shutdown_callback (MetaJackClient *client, JackShutdownCallback shutdown_callback, void *arg)
{
    if (!client->isActive()) {
        shutdownCallbacks[client] = std::make_pair(shutdown_callback, arg);
    }
}

void MetaJackContext::set_info_shutdown_callback (MetaJackClient *client, JackInfoShutdownCallback shutdown_callback, void *arg)
{
    if (!client->isActive()) {
        infoShutdownCallbacks[client] = std::make_pair(shutdown_callback, arg);
    }
}

int MetaJackContext::set_freewheel_callback (MetaJackClient *client, JackFreewheelCallback freewheel_callback, void *arg)
{
    if (client->isActive()) {
        return 1;
    }
    freewheelCallbacks[client] = std::make_pair(freewheel_callback, arg);
    return 0;
}

int MetaJackContext::set_buffer_size_callback (MetaJackClient *client, JackBufferSizeCallback bufsize_callback, void *arg)
{
    if (client->isActive()) {
        return 1;
    }
    bufferSizeCallbacks[client] = std::make_pair(bufsize_callback, arg);
    return 0;
}

int MetaJackContext::set_sample_rate_callback (MetaJackClient *client, JackSampleRateCallback srate_callback, void *arg)
{
    if (client->isActive()) {
        return 1;
    }
    sampleRateCallbacks[client] = std::make_pair(srate_callback, arg);
    return 0;
}

int MetaJackContext::set_client_registration_callback (MetaJackClient *client, JackClientRegistrationCallback registration_callback, void *arg)
{
    if (client->isActive()) {
        return 1;
    }
    clientRegistrationCallbacks[client] = std::make_pair(registration_callback, arg);
    return 0;
}

int MetaJackContext::set_port_registration_callback (MetaJackClient *client, JackPortRegistrationCallback registration_callback, void *arg)
{
    if (client->isActive()) {
        return 1;
    }
    portRegistrationCallbacks[client] = std::make_pair(registration_callback, arg);
    return 0;
}

int MetaJackContext::set_port_connect_callback (MetaJackClient *client, JackPortConnectCallback connect_callback, void *arg)
{
    if (client->isActive()) {
        return 1;
    }
    portConnectCallbacks[client] = std::make_pair(connect_callback, arg);
    return 0;
}

int MetaJackContext::set_port_rename_callback (MetaJackClient *client, JackPortRenameCallback rename_callback, void *arg)
{
    if (client->isActive()) {
        return 1;
    }
    portRenameCallbacks[client] = std::make_pair(rename_callback, arg);
    return 0;
}

int MetaJackContext::set_graph_order_callback (MetaJackClient *client, JackGraphOrderCallback graph_callback, void *arg)
{
    if (client->isActive()) {
        return 1;
    }
    graphOrderCallbacks[client] = std::make_pair(graph_callback, arg);
    return 0;
}

int MetaJackContext::set_xrun_callback (MetaJackClient *client, JackXRunCallback xrun_callback, void *arg)
{
    if (client->isActive()) {
        return 1;
    }
    xRunCallbacks[client] = std::make_pair(xrun_callback, arg);
    return 0;
}

int MetaJackContext::set_freewheel(int onoff)
{
    assert(isActive());
    return jack_set_freewheel(wrapperClient, onoff);
}

int MetaJackContext::set_buffer_size(jack_nframes_t nframes)
{
    assert(isActive());
    return jack_set_buffer_size(wrapperClient, nframes);
}

jack_nframes_t MetaJackContext::get_sample_rate()
{
    assert(isActive());
    return jack_get_sample_rate(wrapperClient);
}

jack_nframes_t MetaJackContext::get_buffer_size()
{
    assert(isActive());
    return jack_get_buffer_size(wrapperClient);
}

float MetaJackContext::get_cpu_load()
{
    assert(isActive());
    return jack_cpu_load(wrapperClient);
}

int MetaJackContext::connectPorts(const std::string &source_port, const std::string &destination_port)
{
    // make sure both ports exist:
    MetaJackPort *source, *dest;
    if (!(source = getPortByName(source_port)) || !(dest = getPortByName(destination_port))) {
        return 1;
    }
    // make sure the ports are active (i.e. belong to active clients):
    if (!source->isActive() || !dest->isActive()) {
        return 1;
    }
    // make sure that one is an input port and one is an output:
    if (source->isInput() == dest->isInput()) {
        return 1;
    }
    // make the connection:
    source->connect(dest);
    return 0;
}

int MetaJackContext::disconnectPorts(const std::string &source_port, const std::string &destination_port)
{
    // make sure both ports exist:
    // make sure both ports exist:
    MetaJackPort *source, *dest;
    if (!(source = getPortByName(source_port)) || !(dest = getPortByName(destination_port))) {
        return 1;
    }
    // check if the ports are connected:
    if (!source->isConnectedTo(dest)) {
        return 1;
    }
    // remove the connection:
    source->disconnect(dest);
    return 0;
}

const char ** MetaJackContext::getPortsbyPattern(const std::string &port_name_pattern, const std::string &type_name_pattern, unsigned long flags)
{
    QRegExp regExpPortNames(port_name_pattern.c_str());
    QRegExp regExpTypeNames(type_name_pattern.c_str());
    std::list<MetaJackPort*> matchingPorts;
    for (std::map<std::string, MetaJackPort*>::iterator i = metaPortsByFullName.begin(); i != metaPortsByFullName.end(); i++) {
        MetaJackPort *port = i->second;
        QString qstringPortName(port->getFullName().c_str());
        QString qstringTypeName(port->getType().c_str());
        if (((port->getFlags() & flags) == flags) && ((port_name_pattern.length() == 0) || regExpPortNames.exactMatch(qstringPortName)) && ((type_name_pattern.length() == 0) || regExpTypeNames.exactMatch(qstringTypeName))) {
            // flags, port name and type match:
            matchingPorts.push_back(port);
        }
    }
    if (matchingPorts.size()) {
        char ** names = new char*[matchingPorts.size() + 1];
        size_t index = 0;
        for (std::list<MetaJackPort*>::iterator i = matchingPorts.begin(); i != matchingPorts.end(); i++, index++) {
            MetaJackPort *port = *i;
            names[index] = new char[port->getFullName().length() + 1];
            memcpy(names[index], port->getFullName().c_str(), port->getFullName().length() + 1);
        }
        names[index] = 0;
        return (const char**)names;
    } else {
        return 0;
    }
}

MetaJackPort * MetaJackContext::getPortByName(const std::string &port_name) const
{
    std::map<std::string, MetaJackPort*>::const_iterator it = metaPortsByFullName.find(port_name);
    if (it != metaPortsByFullName.end()) {
        return it->second;
    } else {
        return 0;
    }
}

MetaJackPort * MetaJackContext::getPortById(jack_port_id_t port_id)
{
    std::map<jack_port_id_t, MetaJackPort*>::iterator it = metaPortsById.find(port_id);
    if (it != metaPortsById.end()) {
        return it->second;
    } else {
        return 0;
    }
}

jack_nframes_t MetaJackContext::get_frames_since_cycle_start() const
{
    assert(isActive());
    return jack_frames_since_cycle_start(wrapperClient);
}

jack_nframes_t MetaJackContext::get_frame_time() const
{
    assert(isActive());
    return jack_frame_time(wrapperClient);
}

jack_nframes_t MetaJackContext::get_last_frame_time() const
{
    assert(isActive());
    return jack_last_frame_time(wrapperClient);
}

jack_time_t MetaJackContext::convert_frames_to_time(jack_nframes_t nframes) const
{
    assert(isActive());
    return jack_frames_to_time(wrapperClient, nframes);
}

jack_nframes_t MetaJackContext::convert_time_to_frames(jack_time_t time) const
{
    assert(isActive());
    return jack_time_to_frames(wrapperClient, time);
}

jack_time_t MetaJackContext::get_time()
{
    return jack_get_time();
}

jack_nframes_t MetaJackContext::midi_get_event_count(void* port_buffer)
{
    MetaJackContextMidiBufferHead *head = (MetaJackContextMidiBufferHead*)port_buffer;
    return head->midiEventCount;
}

int MetaJackContext::midi_event_get(jack_midi_event_t *event, void *port_buffer, jack_nframes_t event_index)
{
    // skip the buffer head:
    char *charBuffer = (char*)port_buffer + sizeof(MetaJackContextMidiBufferHead);;
    charBuffer += sizeof(jack_midi_event_t) * event_index;
    memcpy(event, charBuffer, sizeof(jack_midi_event_t));
    return 0;
}

void MetaJackContext::midi_clear_buffer(void *port_buffer)
{
    MetaJackContextMidiBufferHead *head = (MetaJackContextMidiBufferHead*)port_buffer;
    head->midiDataSize = head->midiEventCount = head->lostMidiEvents = 0;
}

size_t MetaJackContext::midi_max_event_size(void* port_buffer)
{
    MetaJackContextMidiBufferHead *head = (MetaJackContextMidiBufferHead*)port_buffer;
    return head->bufferSize - head->midiEventCount * sizeof(jack_midi_data_t) - head->midiDataSize;
}

jack_midi_data_t* MetaJackContext::midi_event_reserve(void *port_buffer, jack_nframes_t  time, size_t data_size)
{
    MetaJackContextMidiBufferHead *head = (MetaJackContextMidiBufferHead*)port_buffer;
    char *charBuffer = (char*)port_buffer + sizeof(MetaJackContextMidiBufferHead);
    // check if enough space is left:
    if (head->bufferSize >= (head->midiEventCount + 1) * sizeof(jack_midi_data_t) + head->midiDataSize + data_size) {
        jack_midi_event_t event;
        event.time = time;
        event.size = data_size;
        // reserve data_size bytes at the end of the buffer:
        head->midiDataSize += data_size;
        event.buffer = (jack_midi_data_t*)(charBuffer + head->bufferSize - head->midiDataSize);
        charBuffer += sizeof(jack_midi_event_t) * head->midiEventCount;
        head->midiEventCount++;
        memcpy(charBuffer, &event, sizeof(jack_midi_event_t));
        return event.buffer;
    } else {
        return 0;
    }
}

int MetaJackContext::midi_event_write(void *port_buffer, jack_nframes_t time, const jack_midi_data_t *data, size_t data_size)
{
    jack_midi_data_t *buffer = midi_event_reserve(port_buffer, time, data_size);
    if (buffer) {
        memcpy(buffer, data, data_size);
        return 0;
    } else {
        return 1;
    }
}

jack_nframes_t MetaJackContext::midi_get_lost_event_count(void *port_buffer)
{
    MetaJackContextMidiBufferHead *head = (MetaJackContextMidiBufferHead*)port_buffer;
    return head->lostMidiEvents;
}

size_t MetaJackContext::getClientNameSize()
{
    return jack_client_name_size();
}

jack_port_id_t MetaJackContext::createUniquePortId()
{
    return uniquePortId++;
}

void MetaJackContext::free(void* ptr)
{
    if (ptr) {
        char **names = (char**)ptr;
        for (size_t index = 0; names[index]; index++) {
            delete [] names[index];
        }
        delete [] names;
    }
}

void MetaJackContext::registeredClient(MetaJackClient *client)
{
    metaClients.insert(client);
    metaClientsByName[client->getName()] = client;
    invokeJackClientRegistrationCallbacks(client->getName(), true);
}

void MetaJackContext::unregisteredClient(MetaJackClient *client)
{
    threadInitCallbacks.erase(client);
    shutdownCallbacks.erase(client);
    infoShutdownCallbacks.erase(client);
    freewheelCallbacks.erase(client);
    bufferSizeCallbacks.erase(client);
    sampleRateCallbacks.erase(client);
    clientRegistrationCallbacks.erase(client);
    portRegistrationCallbacks.erase(client);
    portConnectCallbacks.erase(client);
    portRenameCallbacks.erase(client);
    graphOrderCallbacks.erase(client);
    xRunCallbacks.erase(client);
    invokeJackClientRegistrationCallbacks(client->getName(), false);
    metaClients.erase(client);
    metaClientsByName.erase(client->getName());
}

void MetaJackContext::activatedClient(MetaJackClient *client)
{
    // tell the process thread that the client is activated:
    MetaJackGraphEvent event;
    event.type = MetaJackGraphEvent::ACTIVATE_CLIENT;
    event.client = client;
    sendGraphChangeEvent(event);
}

void MetaJackContext::deactivatedClient(MetaJackClient *client)
{
    // tell the process thread that the client is deactivated:
    MetaJackGraphEvent event;
    event.type = MetaJackGraphEvent::DEACTIVATE_CLIENT;
    event.client = client;
    sendGraphChangeEvent(event);
}

void MetaJackContext::registeredPort(MetaJackPort *port)
{
    metaPortsById[port->getId()] = port;
    metaPortsByFullName[port->getFullName()] = port;
    if (port->isActive()) {
        // tell the process thread that a new port is registered:
        MetaJackGraphEvent event;
        event.type = MetaJackGraphEvent::REGISTER_PORT;
        event.source = port;
        sendGraphChangeEvent(event);
    }
    // invoke all port registration callbacks:
    invokeJackPortRegistrationCallbacks(port->getId(), true);
}

void MetaJackContext::unregisteredPort(MetaJackPort *port)
{
    if (port->isActive()) {
        // tell the process thread that the port is unregistered:
        MetaJackGraphEvent event;
        event.type = MetaJackGraphEvent::UNREGISTER_PORT;
        event.source = port;
        sendGraphChangeEvent(event);
    }
    // invoke all port registration callbacks:
    invokeJackPortRegistrationCallbacks(port->getId(), false);
    metaPortsById.erase(port->getId());
    metaPortsByFullName.erase(port->getFullName());
}

void MetaJackContext::renamedPort(const std::string &oldFullName, const std::string &newFullName)
{
    std::map<std::string, MetaJackPort*>::iterator i = metaPortsByFullName.find(oldFullName);
    if (i != metaPortsByFullName.end()) {
        MetaJackPort *port = i->second;
        metaPortsByFullName.erase(i);
        metaPortsByFullName[newFullName] = port;
        // invoke the port name change callbacks (is it correct to invoke it with full names?):
        invokeJackPortRenameCallbacks(port->getId(), oldFullName, newFullName);
    }
}

void MetaJackContext::connectedPorts(MetaJackPort *a, MetaJackPort *b)
{
    // tell the process thread that the two ports have been connected:
    MetaJackGraphEvent event;
    event.type = MetaJackGraphEvent::CONNECT_PORTS;
    event.source = a;
    event.dest = b;
    sendGraphChangeEvent(event);
    invokeJackPortConnectCallbacks(a->getId(), b->getId(), true);
}

void MetaJackContext::disconnectedPorts(MetaJackPort *a, MetaJackPort *b)
{
    // tell the process thread that the ports have been disconnected:
    MetaJackGraphEvent event;
    event.type = MetaJackGraphEvent::DISCONNECT_PORTS;
    event.source = a;
    event.dest = b;
    sendGraphChangeEvent(event);
    invokeJackPortConnectCallbacks(a->getId(), b->getId(), false);
}

void MetaJackContext::invokeJackClientRegistrationCallbacks(const std::string &clientName, bool registered)
{
    for (std::map<MetaJackClient*, std::pair<JackClientRegistrationCallback, void*> >::const_iterator i = clientRegistrationCallbacks.begin(); i != clientRegistrationCallbacks.end(); i++) {
        JackClientRegistrationCallback callback = i->second.first;
        void *arg = i->second.second;
        callback(clientName.c_str(), registered ? 1 : 0, arg);
    }
}

void MetaJackContext::invokeJackPortRegistrationCallbacks(jack_port_id_t id, bool registered)
{
    for (std::map<MetaJackClient*, std::pair<JackPortRegistrationCallback, void*> >::iterator i = portRegistrationCallbacks.begin(); i != portRegistrationCallbacks.end(); i++) {
        JackPortRegistrationCallback callback = i->second.first;
        void * arg = i->second.second;
        callback(id, registered ? 1 : 0, arg);
    }
}

void MetaJackContext::invokeJackPortRenameCallbacks(jack_port_id_t id, const std::string &oldName, const std::string &newName)
{
    for (std::map<MetaJackClient*, std::pair<JackPortRenameCallback, void*> >::iterator i = portRenameCallbacks.begin(); i != portRenameCallbacks.end(); i++) {
        JackPortRenameCallback callback = i->second.first;
        void *arg = i->second.second;
        callback(id, oldName.c_str(), newName.c_str(), arg);
    }
}

void MetaJackContext::invokeJackPortConnectCallbacks(jack_port_id_t a, jack_port_id_t b, bool connected)
{
    for (std::map<MetaJackClient*, std::pair<JackPortConnectCallback, void*> >::iterator i = portConnectCallbacks.begin(); i != portConnectCallbacks.end(); i++) {
        JackPortConnectCallback callback = i->second.first;
        void * arg = i->second.second;
        callback(a, a, connected ? 1 : 0, arg);
    }
}

void MetaJackContext::sendGraphChangeEvent(const MetaJackGraphEvent &event)
{
    // write the event to the ring buffer:
    graphChangesRingBuffer.write(event);
    // wait for the process thread to process it:
    waitMutex.lock();
    waitCondition.wait(&waitMutex);
    waitMutex.unlock();
}

int MetaJackContext::process(jack_nframes_t nframes)
{
    // first get all changes to the graph since the last call:
    for (; graphChangesRingBuffer.readSpace(); ) {
        MetaJackGraphEvent event = graphChangesRingBuffer.read();
        if (event.type == MetaJackGraphEvent::ACTIVATE_CLIENT) {
            activeClients.insert(event.client->getTwin());
        } else if (event.type == MetaJackGraphEvent::DEACTIVATE_CLIENT) {
            activeClients.erase(event.client->getTwin());
            event.client->deleteTwin();
        } else if (event.type == MetaJackGraphEvent::REGISTER_PORT) {
            event.source->getTwin();
        } else if (event.type == MetaJackGraphEvent::UNREGISTER_PORT) {
            event.source->deleteTwin();
        } else if (event.type == MetaJackGraphEvent::CONNECT_PORTS) {
            event.source->getTwin()->connect(event.dest->getTwin());
        } else if (event.type == MetaJackGraphEvent::DISCONNECT_PORTS) {
            event.source->getTwin()->disconnect(event.dest->getTwin());
        }
    }
    // wake the other thread(s):
    waitCondition.wakeAll();
    // evaluate the graph structure and call all process callbacks registered by internal clients:
    std::set<MetaJackClient*> unprocessedClients = activeClients;
    bool success = true;
    for (; success && unprocessedClients.size(); ) {
        // get an arbitrary client and process it:
        MetaJackClient *client = *unprocessedClients.begin();
        success = client->process(unprocessedClients, nframes);
    }
    return (success ? 0 : 1);
}

int MetaJackContext::process(jack_nframes_t nframes, void *arg)
{
    return ((MetaJackContext*)arg)->process(nframes);
}

MetaJackDummyInputClient::MetaJackDummyInputClient(MetaJackContext *context, jack_port_t *audioInputPort_, jack_port_t *midiInputPort_) :
    MetaJackClient(context, "system_in", false),
    wrapperAudioInputPort(audioInputPort_),
    wrapperMidiInputPort(midiInputPort_)
{
    audioOutputPort = registerPort("capture_audio", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    midiOutputPort = registerPort("capture_midi", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
    setProcessCallback(process, this);
}

int MetaJackDummyInputClient::process(jack_nframes_t nframes, void *arg)
{
    // the purpose of the dummy input client is to make the wrapper client inputs available to clients inside the wrapper
    MetaJackDummyInputClient *me = (MetaJackDummyInputClient*)arg;
   // copy audio:
    jack_default_audio_sample_t *wrapperAudioInputBuffer = (jack_default_audio_sample_t*)jack_port_get_buffer(me->wrapperAudioInputPort, nframes);
    jack_default_audio_sample_t *audioOutputBuffer = (jack_default_audio_sample_t*)me->audioOutputPort->getBuffer(nframes);
    for (jack_nframes_t i = 0; i < nframes; i++) {
        audioOutputBuffer[i] = wrapperAudioInputBuffer[i];
    }
    // copy midi:
    void *wrapperMidiInputBuffer = jack_port_get_buffer(me->wrapperMidiInputPort, nframes);
    void *midiOutputBuffer = me->midiOutputPort->getBuffer(nframes);
    MetaJackContext::midi_clear_buffer(midiOutputBuffer);
    jack_nframes_t midiEventCount = jack_midi_get_event_count(wrapperMidiInputBuffer);
    for (jack_nframes_t i = 0; i < midiEventCount; i++) {
        jack_midi_event_t event;
        jack_midi_event_get(&event, wrapperMidiInputBuffer, i);
        MetaJackContext::midi_event_write(midiOutputBuffer, event.time, event.buffer, event.size);
    }
    return 0;
}

MetaJackDummyOutputClient::MetaJackDummyOutputClient(MetaJackContext *context, jack_port_t *audioOutputPort_, jack_port_t *midiOutputPort_) :
    MetaJackClient(context, "system_out", false),
    wrapperAudioOutputPort(audioOutputPort_),
    wrapperMidiOutputPort(midiOutputPort_)
{
    audioInputPort = registerPort("playback_audio", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    midiInputPort = registerPort("playback_midi", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
    setProcessCallback(process, this);
}

int MetaJackDummyOutputClient::process(jack_nframes_t nframes, void *arg)
{
    // the purpose of the dummy output client is to make the wrapper client outputs available to clients inside the wrapper
    MetaJackDummyOutputClient *me = (MetaJackDummyOutputClient*)arg;
    // copy audio:
    jack_default_audio_sample_t *audioInputBuffer = (jack_default_audio_sample_t*)me->audioInputPort->getBuffer(nframes);
    jack_default_audio_sample_t *wrapperAudioOutputBuffer = (jack_default_audio_sample_t*)jack_port_get_buffer(me->wrapperAudioOutputPort, nframes);
    for (jack_nframes_t i = 0; i < nframes; i++) {
        wrapperAudioOutputBuffer[i] = audioInputBuffer[i];
    }
    // copy midi:
    void *midiInputBuffer = me->midiInputPort->getBuffer(nframes);
    void *wrapperMidiOutputBuffer = jack_port_get_buffer(me->wrapperMidiOutputPort, nframes);
    jack_midi_clear_buffer(wrapperMidiOutputBuffer);
    jack_nframes_t midiEventCount = MetaJackContext::midi_get_event_count(midiInputBuffer);
    for (jack_nframes_t i = 0; i < midiEventCount; i++) {
        jack_midi_event_t event;
        MetaJackContext::midi_event_get(&event, midiInputBuffer, i);
        jack_midi_event_write(wrapperMidiOutputBuffer, event.time, event.buffer, event.size);
    }
    return 0;
}
