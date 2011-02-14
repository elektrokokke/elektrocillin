#include "metajackcontext.h"
#include <sstream>
#include <list>
#include <cassert>
#include <memory.h>

MetaJackClient::MetaJackClient(MetaJackContext *context_, const std::string &name_, bool processThread_) :
    context(context_),
    name(name_),
    processCallback(0),
    processCallbackArgument(0),
    twin(0),
    processThread(processThread_)
{
    if (!residesInProcessThread()) {
        context->addClient(this);
        context->invokeJackClientRegistrationCallbacks(name, true);
    }
}

MetaJackClient::~MetaJackClient()
{
    if (!residesInProcessThread()) {
        // deactivate the client:
        deactivate();
        context->removeClient(this);
        // invoke all client registration callbacks:
        context->invokeJackClientRegistrationCallbacks(name, false);
    }
    // delete all ports (this will also remove all their connections):
    for (; ports.size(); ) {
        MetaJackPort *port = *ports.begin();
        delete port;
    }
}

size_t MetaJackClient::getNameSize()
{
    return jack_client_name_size();
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
    // tell the process thread that the client is activated:
    MetaJackContextEvent event;
    event.type = MetaJackContextEvent::ACTIVATE_CLIENT;
    event.client = getTwin();
    context->sendGraphChangeEvent(event);
    return true;
}

bool MetaJackClient::deactivate()
{
    if (residesInProcessThread() || !isActive()) {
        return false;
    }
    // disconnect all ports:
    disconnect();
    // tell the process thread that the client is deactivated:
    MetaJackContextEvent event;
    event.type = MetaJackContextEvent::DEACTIVATE_CLIENT;
    event.client = getTwin();
    context->sendGraphChangeEvent(event);
    twin = 0;
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
    if (getContext()->get_port_by_name(full_name)) {
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

void MetaJackClient::addPort(MetaJackPort *port)
{
    ports.insert(port);
    if (!residesInProcessThread()) {
        context->addPort(port);
    }
}

void MetaJackClient::removePort(MetaJackPort *port)
{
    ports.erase(port);
    if (!residesInProcessThread()) {
        context->removePort(port);
    }
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
        getContext()->invokeJackPortRegistrationCallbacks(id, true);
    }
}

MetaJackPort::~MetaJackPort()
{
    // disconnect the port:
    disconnect();
    client->removePort(this);
    if (!residesInProcessThread() && isActive()) {
        // tell the process thread that the port is unregistered:
        MetaJackContextEvent event;
        event.type = MetaJackContextEvent::UNREGISTER_PORT;
        event.source = twin;
        getContext()->sendGraphChangeEvent(event);
        twin = 0;
        // invoke all port registration callbacks:
        getContext()->invokeJackPortRegistrationCallbacks(id, false);
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
    if (getContext()->get_port_by_name(newFullName)) {
        return false;
    }
    std::string oldFullName = getFullName();
    short_name = newName;
    full_name = newFullName;
    // invoke the port name change callbacks (is it correct to invoke it with full names?):
    getContext()->invokeJackPortRenameCallbacks(id, oldFullName, newFullName);
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
    MetaJackPort *port = getContext()->get_port_by_name(port_name);
    return isConnectedTo(port);
}

const char ** MetaJackPort::getConnections() const
{
    if (getConnectionCount()) {
        char ** names = new char*[getConnectionCount() + 1];
        size_t index = 0;
        for (std::set<MetaJackPort*>::iterator i = connectedPorts.begin(); i != connectedPorts.end(); i++, index++) {
            MetaJackPort *connectedPort = *i;
            std::string connectedPortName = connectedPort->getFullName();
            names[index] = new char[connectedPortName.length() + 1];
            memcpy(names[index], connectedPortName.c_str(), connectedPortName.length() + 1);
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
}

void MetaJackPort::connect(MetaJackPort *port)
{
    // TODO: avoid circles in the graph...
    connectedPorts.insert(port);
    port->connectedPorts.insert(this);
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

MetaJackContext * MetaJackContext::instance = &MetaJackContext::instance_;
MetaJackContext MetaJackContext::instance_("meta_jack");

MetaJackContext::MetaJackContext(const std::string &name_) :
    name(name_),
    jackClient(0),
    audioIn(0),
    audioOut(0),
    midiIn(0),
    midiOut(0),
    active(false),
    currentPortId(1),
    graphChangesRingBuffer(1024)
{
    // register at the real JACK server:
    jackClient = jack_client_open(name.c_str(), JackNullOption, 0);
    if (jackClient) {
        // register the process callback:
        jack_set_process_callback(jackClient, process, this);
        // register some ports:
        audioIn = jack_port_register(jackClient, "Audio in", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
        audioOut = jack_port_register(jackClient, "Audio out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
        midiIn = jack_port_register(jackClient, "Midi in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
        midiOut = jack_port_register(jackClient, "Midi out", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
        // activate the client:
        active = !jack_activate(jackClient);
        // get the actual client name:
        name = jack_get_client_name(jackClient);

        // create dummy input and output clients:
        dummyInputClient = client_open("system_in", JackNullOption);
        dummyOutputClient = client_open("system_out", JackNullOption);
        // register their ports:
        dummyInputClient->registerPort("capture_audio", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
        dummyInputClient->registerPort("capture_midi", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
        dummyOutputClient->registerPort("playback_audio", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
        dummyOutputClient->registerPort("playback_midi", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
        // register their process callbacks:
        dummyInputClient->setProcessCallback(processDummyInputClient, this);
        dummyOutputClient->setProcessCallback(processDummyOutputClient, this);
        // activate them:
        dummyInputClient->activate();
        dummyOutputClient->activate();
    }
}

MetaJackContext::~MetaJackContext()
{
    if (jackClient) {
        // first close all clients:
        for (; metaClients.size(); ) {
            MetaJackClient *client = *metaClients.begin();
            delete client;
        }
        // then shutdown the wrapper client:
        jack_client_close(jackClient);
    }
}

/**
  @param options currently only JackUseExactName is considered, all other options are ignored.
  */
MetaJackClient * MetaJackContext::client_open(const std::string &name, jack_options_t options)
{
    // meta clients can only be created if the wrapper client could be activated:
    if (!active) {
        return 0;
    }
    // test if the name is not too long:
    if (name.length() > (size_t)MetaJackClient::getNameSize()) {
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

void MetaJackContext::addClient(MetaJackClient *client)
{
    metaClients.insert(client);
    metaClientsByName[client->getName()] = client;
}

void MetaJackContext::removeClient(MetaJackClient *client)
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
    // remove it from out lists:
    metaClients.erase(client);
    metaClientsByName.erase(client->getName());
}

void MetaJackContext::addPort(MetaJackPort *port)
{
    metaPortsById[port->getId()] = port;
    metaPortsByFullName[port->getFullName()] = port;
}

void MetaJackContext::removePort(MetaJackPort *port)
{
    metaPortsById.erase(port->getId());
    metaPortsByFullName.erase(port->getFullName());
}

int MetaJackContext::get_pid()
{
    assert(active);
    return jack_get_client_pid(name.c_str());
}

pthread_t MetaJackContext::get_thread_id()
{
    assert(active);
    return jack_client_thread_id(jackClient);
}

bool MetaJackContext::is_realtime()
{
    assert(active);
    return jack_is_realtime(jackClient);
}

int MetaJackContext::set_thread_init_callback (MetaJackClient *client, JackThreadInitCallback thread_init_callback, void *arg)
{
    assert(!client->isActive());
    threadInitCallbacks[client] = std::make_pair(thread_init_callback, arg);
    return 0;
}

void MetaJackContext::set_shutdown_callback (MetaJackClient *client, JackShutdownCallback shutdown_callback, void *arg)
{
    assert(!client->isActive());
    shutdownCallbacks[client] = std::make_pair(shutdown_callback, arg);
}

void MetaJackContext::set_info_shutdown_callback (MetaJackClient *client, JackInfoShutdownCallback shutdown_callback, void *arg)
{
    assert(!client->isActive());
    infoShutdownCallbacks[client] = std::make_pair(shutdown_callback, arg);
}

int MetaJackContext::set_freewheel_callback (MetaJackClient *client, JackFreewheelCallback freewheel_callback, void *arg)
{
    assert(!client->isActive());
    freewheelCallbacks[client] = std::make_pair(freewheel_callback, arg);
    return 0;
}

int MetaJackContext::set_buffer_size_callback (MetaJackClient *client, JackBufferSizeCallback bufsize_callback, void *arg)
{
    assert(!client->isActive());
    bufferSizeCallbacks[client] = std::make_pair(bufsize_callback, arg);
    return 0;
}

int MetaJackContext::set_sample_rate_callback (MetaJackClient *client, JackSampleRateCallback srate_callback, void *arg)
{
    assert(!client->isActive());
    sampleRateCallbacks[client] = std::make_pair(srate_callback, arg);
    return 0;
}

int MetaJackContext::set_client_registration_callback (MetaJackClient *client, JackClientRegistrationCallback registration_callback, void *arg)
{
    assert(!client->isActive());
    clientRegistrationCallbacks[client] = std::make_pair(registration_callback, arg);
    return 0;
}

int MetaJackContext::set_port_registration_callback (MetaJackClient *client, JackPortRegistrationCallback registration_callback, void *arg)
{
    assert(!client->isActive());
    portRegistrationCallbacks[client] = std::make_pair(registration_callback, arg);
    return 0;
}

int MetaJackContext::set_port_connect_callback (MetaJackClient *client, JackPortConnectCallback connect_callback, void *arg)
{
    assert(!client->isActive());
    portConnectCallbacks[client] = std::make_pair(connect_callback, arg);
    return 0;
}

int MetaJackContext::set_port_rename_callback (MetaJackClient *client, JackPortRenameCallback rename_callback, void *arg)
{
    assert(!client->isActive());
    portRenameCallbacks[client] = std::make_pair(rename_callback, arg);
    return 0;
}

int MetaJackContext::set_graph_order_callback (MetaJackClient *client, JackGraphOrderCallback graph_callback, void *arg)
{
    assert(!client->isActive());
    graphOrderCallbacks[client] = std::make_pair(graph_callback, arg);
    return 0;
}

int MetaJackContext::set_xrun_callback (MetaJackClient *client, JackXRunCallback xrun_callback, void *arg)
{
    assert(!client->isActive());
    xRunCallbacks[client] = std::make_pair(xrun_callback, arg);
    return 0;
}

int MetaJackContext::set_freewheel(int onoff)
{
    assert(active);
    return jack_set_freewheel(jackClient, onoff);
}

int MetaJackContext::set_buffer_size(jack_nframes_t nframes)
{
    assert(active);
    return jack_set_buffer_size(jackClient, nframes);
}

jack_nframes_t MetaJackContext::get_sample_rate()
{
    assert(active);
    return jack_get_sample_rate(jackClient);
}

jack_nframes_t MetaJackContext::get_buffer_size()
{
    assert(active);
    return jack_get_buffer_size(jackClient);
}

float MetaJackContext::get_cpu_load()
{
    assert(active);
    return jack_cpu_load(jackClient);
}

int MetaJackContext::port_connect(const std::string &source_port, const std::string &destination_port)
{
    // make sure both ports exist:
    MetaJackPort *source, *dest;
    if (!(source = get_port_by_name(source_port)) || !(dest = get_port_by_name(destination_port))) {
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
    // tell the process thread that the two ports have been connected:
    MetaJackContextEvent event;
    event.type = MetaJackContextEvent::CONNECT_PORTS;
    event.source = source->getTwin();
    event.dest = dest->getTwin();
    sendGraphChangeEvent(event);
    invokeJackPortConnectCallbacks(source->getId(), dest->getId(), true);
    return 0;
}

int MetaJackContext::port_disconnect(const std::string &source_port, const std::string &destination_port)
{
    // make sure both ports exist:
    // make sure both ports exist:
    MetaJackPort *source, *dest;
    if (!(source = get_port_by_name(source_port)) || !(dest = get_port_by_name(destination_port))) {
        return 1;
    }
    // check if the ports are connected:
    if (!source->isConnectedTo(dest)) {
        return 1;
    }
    // remove the connection:
    source->disconnect(dest);
    // tell the process thread that the ports have been disconnected:
    MetaJackContextEvent event;
    event.type = MetaJackContextEvent::DISCONNECT_PORTS;
    event.source = source->getTwin();
    event.dest = dest->getTwin();
    sendGraphChangeEvent(event);
    invokeJackPortConnectCallbacks(source->getId(), dest->getId(), false);
    return 0;
}

const char ** MetaJackContext::get_ports(const std::string &port_name_pattern, const std::string &type_name_pattern, unsigned long flags)
{
    // TODO: implement port lookup using regular expressions
    return 0;
}

MetaJackPort * MetaJackContext::get_port_by_name(const std::string &port_name) const
{
    std::map<std::string, MetaJackPort*>::const_iterator it = metaPortsByFullName.find(port_name);
    if (it != metaPortsByFullName.end()) {
        return it->second;
    } else {
        return 0;
    }
}

MetaJackPort * MetaJackContext::get_port_by_id(jack_port_id_t port_id)
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
    assert(active);
    return jack_frames_since_cycle_start(jackClient);
}

jack_nframes_t MetaJackContext::get_frame_time() const
{
    assert(active);
    return jack_frame_time(jackClient);
}

jack_nframes_t MetaJackContext::get_last_frame_time() const
{
    assert(active);
    return jack_last_frame_time(jackClient);
}

jack_time_t MetaJackContext::convert_frames_to_time(jack_nframes_t nframes) const
{
    assert(active);
    return jack_frames_to_time(jackClient, nframes);
}

jack_nframes_t MetaJackContext::convert_time_to_frames(jack_time_t time) const
{
    assert(active);
    return jack_time_to_frames(jackClient, time);
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

jack_port_id_t MetaJackContext::createUniquePortId()
{
    return currentPortId++;
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

void MetaJackContext::sendGraphChangeEvent(const MetaJackContextEvent &event)
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
        MetaJackContextEvent event = graphChangesRingBuffer.read();
        if (event.type == MetaJackContextEvent::ACTIVATE_CLIENT) {
            metaClientsForProcess.insert(event.client);
        } else if (event.type == MetaJackContextEvent::DEACTIVATE_CLIENT) {
            metaClientsForProcess.erase(event.client);
            delete event.client;
        } else if (event.type == MetaJackContextEvent::UNREGISTER_PORT) {
            delete event.source;
        } else if (event.type == MetaJackContextEvent::CONNECT_PORTS) {
            event.source->connect(event.dest);
        } else if (event.type == MetaJackContextEvent::DISCONNECT_PORTS) {
            event.source->disconnect(event.dest);
        }
    }
    // wake the other thread(s):
    waitCondition.wakeAll();
    // TODO: evaluate the graph structure and call all process callbacks registered by internal clients
    std::set<MetaJackClient*> unprocessedClients = metaClientsForProcess;
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

int MetaJackContext::processDummyInputClient(jack_nframes_t nframes, void *arg)
{
    MetaJackContext *context = (MetaJackContext*)arg;
    // copy from the real inputs to the dummy client outputs:
    MetaJackPort *audioOutputPort = context->get_port_by_name("system_in:capture_audio");
    MetaJackPort *midiOutputPort = context->get_port_by_name("system_in:capture_midi");
    assert(audioOutputPort && midiOutputPort);
    // copy audio:
    jack_default_audio_sample_t *audioOutputBuffer = (jack_default_audio_sample_t*)audioOutputPort->getBuffer(nframes);
    jack_default_audio_sample_t *audioInputBuffer = (jack_default_audio_sample_t*)jack_port_get_buffer(context->audioIn, nframes);
    for (jack_nframes_t i = 0; i < nframes; i++) {
        audioOutputBuffer[i] = audioInputBuffer[i];
    }
    // copy midi:
    void *midiOutputBuffer = midiOutputPort->getBuffer(nframes);
    void *midiInputBuffer = jack_port_get_buffer(context->midiIn, nframes);
    midi_clear_buffer(midiOutputBuffer);
    jack_nframes_t midiEventCount = jack_midi_get_event_count(midiInputBuffer);
    for (jack_nframes_t i = 0; i < midiEventCount; i++) {
        jack_midi_event_t event;
        jack_midi_event_get(&event, midiInputBuffer, i);
        midi_event_write(midiOutputBuffer, event.time, event.buffer, event.size);
    }
    return 0;
}

int MetaJackContext::processDummyOutputClient(jack_nframes_t nframes, void *arg)
{
    MetaJackContext *context = (MetaJackContext*)arg;
    // copy from the real inputs to the dummy client outputs:
    MetaJackPort *audioInputPort = context->get_port_by_name("system_out:playback_audio");
    MetaJackPort *midiInputPort = context->get_port_by_name("system_out:playback_midi");
    assert(audioInputPort && midiInputPort);
    // copy audio:
    jack_default_audio_sample_t *audioInputBuffer = (jack_default_audio_sample_t*)audioInputPort->getBuffer(nframes);
    jack_default_audio_sample_t *audioOutputBuffer = (jack_default_audio_sample_t*)jack_port_get_buffer(context->audioOut, nframes);
    for (jack_nframes_t i = 0; i < nframes; i++) {
        audioOutputBuffer[i] = audioInputBuffer[i];
    }
    // copy midi:
    void *midiInputBuffer = midiInputPort->getBuffer(nframes);
    void *midiOutputBuffer = jack_port_get_buffer(context->midiOut, nframes);
    jack_midi_clear_buffer(midiOutputBuffer);
    jack_nframes_t midiEventCount = midi_get_event_count(midiInputBuffer);
    for (jack_nframes_t i = 0; i < midiEventCount; i++) {
        jack_midi_event_t event;
        midi_event_get(&event, midiInputBuffer, i);
        jack_midi_event_write(midiOutputBuffer, event.time, event.buffer, event.size);
    }
    return 0;
}
