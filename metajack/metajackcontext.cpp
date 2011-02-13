#include "metajackcontext.h"
#include <sstream>
#include <cassert>

_meta_jack_client::_meta_jack_client(MetaJackContext *context_, const std::string &name_) :
    context(context_),
    name(name_),
    processCallback(0),
    processCallbackArgument(0),
    twin(0)
{}

_meta_jack_client::~_meta_jack_client()
{
    // delete all ports (this will also remove all their connections):
    for (std::set<_meta_jack_port*>::iterator i = ports.begin(); i != ports.end(); i++) {
        _meta_jack_port *port = *i;
        delete port;
    }
}

void _meta_jack_client::disconnect()
{
    // disconnect all ports:
    for (std::set<_meta_jack_port*>::iterator i = ports.begin(); i != ports.end(); i++) {
        _meta_jack_port *port = *i;
        port->disconnect();
    }
}

_meta_jack_client * _meta_jack_client::createTwin()
{
    if (twin) {
        return twin;
    }
    twin = new _meta_jack_client(context, name);
    twin->processCallback = processCallback;
    twin->processCallbackArgument = processCallbackArgument;
    // duplicate the ports:
    for (std::set<_meta_jack_port*>::iterator i = ports.begin(); i != ports.end(); i++) {
        _meta_jack_port *port = *i;
        createPortTwin(port);
    }
    return twin;
}

_meta_jack_port * _meta_jack_client::createPortTwin(_meta_jack_port *port)
{
    assert(twin);
    _meta_jack_port * portTwin = port->createTwin();
    twin->ports.insert(portTwin);
    twin->portsByShortName[portTwin->short_name] = portTwin;
    return portTwin;
}

_meta_jack_port::_meta_jack_port(_meta_jack_client *client_, jack_port_id_t id_, const std::string &short_name_, const std::string &type_, int flags_) :
    client(client_),
    id(id_),
    short_name(short_name_),
    type(type_),
    flags(flags_),
    bufferSize(0),
    buffer(0)
{}

_meta_jack_port::~_meta_jack_port()
{
    // disconnect the port:
    disconnect();
    // remove it from the client's lists:
    client->ports.erase(this);
    client->portsByShortName.erase(short_name);
    // delete the buffer:
    delete [] buffer;
}

std::string _meta_jack_port::getFullName()
{
    std::stringstream stream;
    stream << client->name << ":" << short_name;
    return stream.str();
}

void _meta_jack_port::disconnect()
{
    for (std::set<_meta_jack_port*>::iterator i = connectedPorts.begin(); i != connectedPorts.end(); i++) {
        _meta_jack_port *connectedPort = *i;
        disconnect(connectedPort);
    }
}

void _meta_jack_port::disconnect(_meta_jack_port *port)
{
    connectedPorts.erase(port);
    port->connectedPorts.erase(this);
}

void _meta_jack_port::connect(_meta_jack_port *port)
{
    // TODO: avoid circles in the graph...
    connectedPorts.insert(port);
    port->connectedPorts.insert(this);
}

bool _meta_jack_port::isInput()
{
    return (flags & JackPortIsInput);
}

_meta_jack_port * _meta_jack_port::createTwin()
{
    if (twin) {
        return twin;
    }
    twin = new _meta_jack_port(client->twin, id, short_name, type, flags);
    twin->aliases[0] = aliases[0];
    twin->aliases[1] = aliases[1];
    // duplicate the connections:
    for (std::set<_meta_jack_port*>::iterator i = connectedPorts.begin(); i != connectedPorts.end(); i++) {
        _meta_jack_port *connectedPort = *i;
        twin->connectedPorts.insert(connectedPort->createTwin());
    }
    return twin;
}

void _meta_jack_port::clearBuffer()
{
    if (type == JACK_DEFAULT_AUDIO_TYPE) {
        // clearing means setting everything to zero:
        memset(buffer, 0, bufferSize);
    } else if (type == JACK_DEFAULT_MIDI_TYPE) {
        client->context->midi_clear_buffer(buffer);
    }
}

void _meta_jack_port::mergeBuffers(_meta_jack_port *destination_port)
{
    assert(type == destination_port->type);
    assert(bufferSize == destination_port->bufferSize);
    // merge this port's buffer into the buffer of the given port:
    if (type == JACK_DEFAULT_AUDIO_TYPE) {
        // merging means adding in this case:
        jack_default_audio_sample_t *sourceBuffer = (jack_default_audio_sample_t*)buffer;
        jack_default_audio_sample_t *destBuffer = (jack_default_audio_sample_t*)destination_port->buffer;
        size_t nframes = bufferSize / sizeof(jack_default_audio_sample_t);
        for (jack_nframes_t i = 0; i < nframes; i++) {
            destBuffer[i] += sourceBuffer[i];
        }
    } else if (type == JACK_DEFAULT_MIDI_TYPE) {
        // TODO
    }
}

MetaJackContext * MetaJackContext::instance = new MetaJackContext("meta_jack");

MetaJackContext::MetaJackContext(const std::string &name_) :
    name(name_),
    client(0),
    audioIn(0),
    audioOut(0),
    midiIn(0),
    midiOut(0),
    active(false),
    currentPortId(1),
    graphChangesRingBuffer(1024)
{
    // register at the real JACK server:
    client = jack_client_open(name.c_str(), JackNullOption, 0);
    if (client) {
        // register the process callback:
        jack_set_process_callback(client, process, this);
        // register some ports:
        audioIn = jack_port_register(client, "Audio in", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
        audioOut = jack_port_register(client, "Audio out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
        midiIn = jack_port_register(client, "Midi in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
        midiOut = jack_port_register(client, "Midi out", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
        // activate the client:
        active = !jack_activate(client);
        // get the actual client name:
        name = jack_get_client_name(client);

        // create dummy input and output clients:
        dummyInputClient = client_open("system_in", JackNullOption);
        dummyOutputClient = client_open("system_out", JackNullOption);
        // register their ports:
        port_register(dummyInputClient, "capture_audio", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
        port_register(dummyInputClient, "capture_midi", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
        port_register(dummyOutputClient, "playback_audio", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
        port_register(dummyOutputClient, "playback_midi", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
        // register their process callbacks:
        set_process_callback(dummyInputClient, processDummyInputClient, this);
        set_process_callback(dummyOutputClient, processDummyOutputClient, this);
        // activate them:
        client_activate(dummyInputClient);
        client_activate(dummyOutputClient);
    }
}

MetaJackContext::~MetaJackContext()
{
    if (client) {
        // first close all clients (this will also delete them and any twins):
        for (std::set<_meta_jack_client*>::iterator i = metaClients.begin(); i != metaClients.end(); i++) {
            _meta_jack_client *metaClient = *i;
            client_close(metaClient);
        }
        // then shutdown the wrapper client:
        jack_client_close(client);
    }
}

/**
  @param options currently only JackUseExactName is considered, all other options are ignored.
  */
meta_jack_client_t * MetaJackContext::client_open(const std::string &name, jack_options_t options)
{
    // meta clients can only be created if the wrapper client could be activated:
    if (!active) {
        return 0;
    }
    // test if the name is not too long:
    if (name.length() > (size_t)client_get_name_size()) {
        return 0;
    }
    // test if the requested name is already taken:
    bool nameIsTaken = (metaClientsByName.find(name) != metaClientsByName.end());
    // if the name is taken but the JackUseExactName flag is given, we can't continue:
    if (nameIsTaken && (options & JackUseExactName)) {
        return 0;
    }
    // create a new meta client:
    _meta_jack_client *metaClient = new _meta_jack_client(this, name);
    // if the name is taken, append a suffix:
    for (int suffix = 2; nameIsTaken; suffix++) {
        std::stringstream stream;
        stream << name << suffix;
        metaClient->name = stream.str();
        nameIsTaken = (metaClientsByName.find(metaClient->name) != metaClientsByName.end());
    }
    // put the client in our lists:
    metaClients.insert(metaClient);
    metaClientsByName[metaClient->name] = metaClient;
    // invoke all client registration callbacks:
    for (std::map<meta_jack_client_t*, std::pair<JackClientRegistrationCallback, void*> >::iterator i = clientRegistrationCallbacks.begin(); i != clientRegistrationCallbacks.end(); i++) {
        JackClientRegistrationCallback callback = i->second.first;
        void * arg = i->second.second;
        callback(metaClient->name.c_str(), 1, arg);
    }
    return metaClient;
}

int MetaJackContext::client_close(_meta_jack_client *metaClient)
{
    // remove all of the client's callbacks:
    threadInitCallbacks.erase(metaClient);
    shutdownCallbacks.erase(metaClient);
    infoShutdownCallbacks.erase(metaClient);
    freewheelCallbacks.erase(metaClient);
    bufferSizeCallbacks.erase(metaClient);
    sampleRateCallbacks.erase(metaClient);
    clientRegistrationCallbacks.erase(metaClient);
    portRegistrationCallbacks.erase(metaClient);
    portConnectCallbacks.erase(metaClient);
    portRenameCallbacks.erase(metaClient);
    graphOrderCallbacks.erase(metaClient);
    xRunCallbacks.erase(metaClient);
    // first deactivate the client:
    client_deactivate(metaClient);
    // invoke all client registration callbacks:
    for (std::map<meta_jack_client_t*, std::pair<JackClientRegistrationCallback, void*> >::iterator i = clientRegistrationCallbacks.begin(); i != clientRegistrationCallbacks.end(); i++) {
        JackClientRegistrationCallback callback = i->second.first;
        void * arg = i->second.second;
        callback(metaClient->name.c_str(), 0, arg);
    }
    // remove the meta client from our sets:
    metaClientsByName.erase(metaClient->name);
    metaClients.erase(metaClient);
    // remove all of its ports from our lists:
    for (std::set<_meta_jack_port*>::iterator i = metaClient->ports.begin(); i != metaClient->ports.end(); i++) {
        _meta_jack_port *port = *i;
        metaPortsById.erase(port->id);
        metaPortsByFullName.erase(port->getFullName());
    }
    // delete the meta client (this should also remove all ports and their connections):
    delete metaClient;
    return 0;
}

int MetaJackContext::client_get_name_size()
{
    return jack_client_name_size();
}

const std::string & MetaJackContext::client_get_name(_meta_jack_client *client) const
{
    return client->name;
}

int MetaJackContext::client_activate(_meta_jack_client *metaClient)
{
    if (metaClient->twin) {
        return 1;
    }
    // signal the process thread:
    MetaJackContextEvent event;
    event.type = MetaJackContextEvent::ACTIVATE_CLIENT;
    event.client = metaClient->createTwin();
    graphChangesRingBuffer.write(event);
    waitMutex.lock();
    waitCondition.wait(&waitMutex);
    waitMutex.unlock();
    return 0;
}

int MetaJackContext::client_deactivate(_meta_jack_client *metaClient)
{
    if (!metaClient->twin) {
        return 1;
    }
    // disconnect all ports:
    metaClient->disconnect();
    // signal the process thread:
    MetaJackContextEvent event;
    event.type = MetaJackContextEvent::DEACTIVATE_CLIENT;
    event.client = metaClient->twin;
    graphChangesRingBuffer.write(event);
    waitMutex.lock();
    waitCondition.wait(&waitMutex);
    waitMutex.unlock();
    delete metaClient->twin;
    metaClient->twin = 0;
    return 0;
}

int MetaJackContext::get_pid()
{
    assert(active);
    return jack_get_client_pid(name.c_str());
}

pthread_t MetaJackContext::get_thread_id()
{
    assert(active);
    return jack_client_thread_id(client);
}

bool MetaJackContext::is_realtime()
{
    assert(active);
    return jack_is_realtime(client);
}

int MetaJackContext::set_thread_init_callback (_meta_jack_client *client, JackThreadInitCallback thread_init_callback, void *arg)
{
    assert(!client->twin);
    threadInitCallbacks[client] = std::make_pair(thread_init_callback, arg);
    return 0;
}

void MetaJackContext::set_shutdown_callback (_meta_jack_client *client, JackShutdownCallback shutdown_callback, void *arg)
{
    assert(!client->twin);
    shutdownCallbacks[client] = std::make_pair(shutdown_callback, arg);
}

void MetaJackContext::set_info_shutdown_callback (_meta_jack_client *client, JackInfoShutdownCallback shutdown_callback, void *arg)
{
    assert(!client->twin);
    infoShutdownCallbacks[client] = std::make_pair(shutdown_callback, arg);
}

int MetaJackContext::set_process_callback (_meta_jack_client *client, JackProcessCallback process_callback, void *arg)
{
    assert(!client->twin);
    client->processCallback = process_callback;
    client->processCallbackArgument = arg;
    return 0;
}

int MetaJackContext::set_freewheel_callback (_meta_jack_client *client, JackFreewheelCallback freewheel_callback, void *arg)
{
    assert(!client->twin);
    freewheelCallbacks[client] = std::make_pair(freewheel_callback, arg);
    return 0;
}

int MetaJackContext::set_buffer_size_callback (_meta_jack_client *client, JackBufferSizeCallback bufsize_callback, void *arg)
{
    assert(!client->twin);
    bufferSizeCallbacks[client] = std::make_pair(bufsize_callback, arg);
    return 0;
}

int MetaJackContext::set_sample_rate_callback (_meta_jack_client *client, JackSampleRateCallback srate_callback, void *arg)
{
    assert(!client->twin);
    sampleRateCallbacks[client] = std::make_pair(srate_callback, arg);
    return 0;
}

int MetaJackContext::set_client_registration_callback (_meta_jack_client *client, JackClientRegistrationCallback registration_callback, void *arg)
{
    assert(!client->twin);
    clientRegistrationCallbacks[client] = std::make_pair(registration_callback, arg);
    return 0;
}

int MetaJackContext::set_port_registration_callback (_meta_jack_client *client, JackPortRegistrationCallback registration_callback, void *arg)
{
    assert(!client->twin);
    portRegistrationCallbacks[client] = std::make_pair(registration_callback, arg);
    return 0;
}

int MetaJackContext::set_port_connect_callback (_meta_jack_client *client, JackPortConnectCallback connect_callback, void *arg)
{
    assert(!client->twin);
    portConnectCallbacks[client] = std::make_pair(connect_callback, arg);
    return 0;
}

int MetaJackContext::set_port_rename_callback (_meta_jack_client *client, JackPortRenameCallback rename_callback, void *arg)
{
    assert(!client->twin);
    portRenameCallbacks[client] = std::make_pair(rename_callback, arg);
    return 0;
}

int MetaJackContext::set_graph_order_callback (_meta_jack_client *client, JackGraphOrderCallback graph_callback, void *arg)
{
    assert(!client->twin);
    graphOrderCallbacks[client] = std::make_pair(graph_callback, arg);
    return 0;
}

int MetaJackContext::set_xrun_callback (_meta_jack_client *client, JackXRunCallback xrun_callback, void *arg)
{
    assert(!client->twin);
    xRunCallbacks[client] = std::make_pair(xrun_callback, arg);
    return 0;
}

int MetaJackContext::set_freewheel(int onoff)
{
    assert(active);
    return jack_set_freewheel(client, onoff);
}

int MetaJackContext::set_buffer_size(jack_nframes_t nframes)
{
    assert(active);
    return jack_set_buffer_size(client, nframes);
}

jack_nframes_t MetaJackContext::get_sample_rate()
{
    assert(active);
    return jack_get_sample_rate(client);
}

jack_nframes_t MetaJackContext::get_buffer_size()
{
    assert(active);
    return jack_get_buffer_size(client);
}

float MetaJackContext::get_cpu_load()
{
    assert(active);
    return jack_cpu_load(client);
}

_meta_jack_port * MetaJackContext::port_register(_meta_jack_client *metaClient, const std::string &port_name, const std::string &port_type, unsigned long flags, unsigned long buffer_size)
{
    // check if the given port name is not too long:
    if (metaClient->name.length() + port_name.length() + 1 > (size_t)jack_port_name_size()) {
        return 0;
    }
    // check if the given port name is already taken:
    if (metaClient->portsByShortName.find(port_name) != metaClient->portsByShortName.end()) {
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
    jack_port_id_t id = currentPortId++;
    // create a new port:
    _meta_jack_port * port = new _meta_jack_port(metaClient, id, port_name, port_type, flags);
    // add the port to all necessary lists:
    metaPortsById[id] = port;
    metaPortsByFullName[port->getFullName()] = port;
    metaClient->ports.insert(port);
    metaClient->portsByShortName[port_name] = port;
    // invoke all port registration callbacks:
    for (std::map<meta_jack_client_t*, std::pair<JackPortRegistrationCallback, void*> >::iterator i = portRegistrationCallbacks.begin(); i != portRegistrationCallbacks.end(); i++) {
        JackPortRegistrationCallback callback = i->second.first;
        void * arg = i->second.second;
        callback(id, 1, arg);
    }
    return port;
}

int MetaJackContext::port_unregister(_meta_jack_port *port)
{
    // signal the process thread:
    if (port->twin) {
        MetaJackContextEvent event;
        event.type = MetaJackContextEvent::UNREGISTER_PORT;
        event.source = port->twin;
        graphChangesRingBuffer.write(event);
        waitMutex.lock();
        waitCondition.wait(&waitMutex);
        waitMutex.unlock();
        delete port->twin;
        port->twin = 0;
    }
    // invoke all port registration callbacks:
    for (std::map<meta_jack_client_t*, std::pair<JackPortRegistrationCallback, void*> >::iterator i = portRegistrationCallbacks.begin(); i != portRegistrationCallbacks.end(); i++) {
        JackPortRegistrationCallback callback = i->second.first;
        void * arg = i->second.second;
        callback(port->id, 0, arg);
    }
    // remove the port from our list:
    metaPortsById.erase(port->id);
    metaPortsByFullName.erase(port->getFullName());
    // delete the port (this should also remove all its connections):
    delete port;
    return 0;
}

void * MetaJackContext::port_get_buffer(_meta_jack_port *port, jack_nframes_t nframes)
{
    // resize the buffer if necessary:
    if (nframes > port->bufferSize) {
        if (port->buffer) {
            delete [] port->buffer;
        }
        port->bufferSize = nframes;
        port->buffer = new char[nframes * sizeof(jack_default_audio_sample_t)];
        // if this is a MIDI port, write its size to the begin of the buffer:
        if (port->type == JACK_DEFAULT_MIDI_TYPE) {
            size_t adjustedBufferSize = port->bufferSize - sizeof(size_t) * 3;
            MetaJackContextMidiBufferHead *head = (MetaJackContextMidiBufferHead*)port->buffer;
            head->bufferSize = adjustedBufferSize;
            head->midiDataSize = head->midiEventCount = 0;
        }
    }
    return port->buffer;
}

const std::string & MetaJackContext::port_get_name(const _meta_jack_port *port)
{
    std::stringstream stream;
    stream << port->client->name << ":" << port->short_name;
    const_cast<_meta_jack_port*>(port)->full_name = stream.str();
    return port->full_name;
}

const std::string & MetaJackContext::port_get_short_name(const _meta_jack_port *port)
{
    return port->short_name;
}

int MetaJackContext::port_get_flags(const _meta_jack_port *port)
{
    return port->flags;
}

const std::string & MetaJackContext::port_get_type(const _meta_jack_port *port)
{
    return port->type;
}

jack_port_type_id_t MetaJackContext::port_get_type_id(const _meta_jack_port *port)
{
    // TODO: find out what this is good for...
    if (port->type == JACK_DEFAULT_AUDIO_TYPE) {
        return 1;
    } else if (port->type == JACK_DEFAULT_MIDI_TYPE) {
        return 2;
    } else {
        return 0;
    }
}

bool MetaJackContext::port_is_mine (const _meta_jack_client *client, const _meta_jack_port *port)
{
    return port->client == client;
}

int MetaJackContext::port_get_connection_count(const _meta_jack_port *port)
{
    return port->connectedPorts.size();
}

bool MetaJackContext::port_is_connected_to(const _meta_jack_port *port, const std::string &port_name)
{
    std::map<std::string, _meta_jack_port*>::iterator port_it = metaPortsByFullName.find(port_name);
    if (port_it == metaPortsByFullName.end()) {
        return false;
    } else{
        _meta_jack_port *otherPort = port_it->second;
        return port->connectedPorts.find(otherPort) != port->connectedPorts.end();
    }
}

const char ** MetaJackContext::port_get_connections(const _meta_jack_port *port)
{
    if (port->connectedPorts.size()) {
        char ** names = new char*[port->connectedPorts.size() + 1];
        size_t index = 0;
        for (std::set<_meta_jack_port*>::iterator i = port->connectedPorts.begin(); i != port->connectedPorts.end(); i++, index++) {
            _meta_jack_port *connectedPort = *i;
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

const char ** MetaJackContext::port_get_all_connections(const _meta_jack_port *port)
{
    // until the difference is clear (currently it makes no difference), just call the other method:
    return port_get_connections(port);
}

jack_nframes_t MetaJackContext::port_get_latency(_meta_jack_port *port)
{
    return 0;
}

jack_nframes_t MetaJackContext::port_get_total_latency(_meta_jack_port *port)
{
    // TODO figure out how to do this best, until then return 0:
    return 0;
}

int MetaJackContext::port_set_name(_meta_jack_port *port, const std::string &port_name)
{
    // make sure that the long port name is not longer than possible:
    std::string new_name = port_name;
    size_t maxSize = jack_port_name_size();
    if (port_name.length() + port->client->name.length() + 1 > maxSize) {
        // truncate the name:
        new_name.resize(maxSize - port->client->name.length() - 1);
    }
    // check if the port name is already taken:
    if (port->client->portsByShortName.find(new_name) != port->client->portsByShortName.end()) {
        return 1;
    }
    std::string old_name = port->short_name;
    port->short_name = new_name;
    // invoke the port name change callbacks:
    for (std::map<meta_jack_client_t*, std::pair<JackPortRenameCallback, void*> >::iterator i = portRenameCallbacks.begin(); i != portRenameCallbacks.end(); i++) {
        JackPortRenameCallback callback = i->second.first;
        void *arg = i->second.second;
        callback(port->id, old_name.c_str(), new_name.c_str(), arg);
    }
    return 0;
}

int MetaJackContext::port_set_alias(_meta_jack_port *port, const std::string &alias)
{
    // TODO
    return 1;
}

int MetaJackContext::port_unset_alias(_meta_jack_port *port, const std::string &alias)
{
    // TODO
    return 1;
}

int MetaJackContext::port_get_aliases(const _meta_jack_port *port, char* const aliases[2])
{
    // TODO
    return 0;
}

int MetaJackContext::port_connect(const std::string &source_port, const std::string &destination_port)
{
    // make sure both ports exist:
    std::map<std::string, _meta_jack_port*>::iterator source_it = metaPortsByFullName.find(source_port);
    std::map<std::string, _meta_jack_port*>::iterator destination_it = metaPortsByFullName.find(destination_port);
    if ((source_it == metaPortsByFullName.end()) || (destination_it == metaPortsByFullName.end())) {
        return 1;
    }
    _meta_jack_port *source = source_it->second;
    _meta_jack_port *dest = destination_it->second;
    // make sure the ports' clients are active:
    if (!source->client->twin || !dest->client->twin) {
        return 1;
    }
    // make sure that one is an input port and one is an output:
    if (source->isInput() == dest->isInput()) {
        return 1;
    }
    // make the connection:
    source->connect(dest);
    // signal the process thread:
    MetaJackContextEvent event;
    event.type = MetaJackContextEvent::CONNECT_PORTS;
    event.source = source->createTwin();
    event.dest = dest->createTwin();
    graphChangesRingBuffer.write(event);
    waitMutex.lock();
    waitCondition.wait(&waitMutex);
    waitMutex.unlock();
    // invoke all port connection callbacks:
    for (std::map<meta_jack_client_t*, std::pair<JackPortConnectCallback, void*> >::iterator i = portConnectCallbacks.begin(); i != portConnectCallbacks.end(); i++) {
        JackPortConnectCallback callback = i->second.first;
        void * arg = i->second.second;
        callback(source->id, dest->id, 1, arg);
    }
    return 0;
}

int MetaJackContext::port_disconnect(const std::string &source_port, const std::string &destination_port)
{
    // make sure both ports exist:
    std::map<std::string, _meta_jack_port*>::iterator source_it = metaPortsByFullName.find(source_port);
    std::map<std::string, _meta_jack_port*>::iterator destination_it = metaPortsByFullName.find(destination_port);
    if ((source_it == metaPortsByFullName.end()) || (destination_it == metaPortsByFullName.end())) {
        return 1;
    }
    _meta_jack_port *source = source_it->second;
    _meta_jack_port *dest = destination_it->second;
    // check if the ports are connected:
    if (source->connectedPorts.find(dest) == source->connectedPorts.end()) {
        return 1;
    }
    // remove the connection:
    source->disconnect(dest);
    // signal the process thread:
    MetaJackContextEvent event;
    event.type = MetaJackContextEvent::DISCONNECT_PORTS;
    event.source = source->twin;
    event.dest = dest->twin;
    graphChangesRingBuffer.write(event);
    waitMutex.lock();
    waitCondition.wait(&waitMutex);
    waitMutex.unlock();
    if (source->connectedPorts.size() == 0) {
        delete source->twin;
        source->twin = 0;
    }
    if (dest->connectedPorts.size() == 0) {
        delete dest->twin;
        dest->twin = 0;
    }
    // invoke all port connection callbacks:
    for (std::map<meta_jack_client_t*, std::pair<JackPortConnectCallback, void*> >::iterator i = portConnectCallbacks.begin(); i != portConnectCallbacks.end(); i++) {
        JackPortConnectCallback callback = i->second.first;
        void * arg = i->second.second;
        callback(source->id, dest->id, 0, arg);
    }
    return 1;
}

int MetaJackContext::port_disconnect(meta_jack_client_t *client, meta_jack_port_t *port)
{
    // documentation of this function is very strange, what does it do?
    return 1;
}

int MetaJackContext::port_get_name_size()
{
    return jack_port_name_size();
}

int MetaJackContext::port_get_type_size()
{
    return jack_port_type_size();
}

const char ** MetaJackContext::get_ports(const std::string &port_name_pattern, const std::string &type_name_pattern, unsigned long flags)
{
    // TODO: implement port lookup using regular expressions
    return 0;
}

meta_jack_port_t * MetaJackContext::get_port_by_name(const std::string &port_name)
{
    std::map<std::string, _meta_jack_port*>::iterator it = metaPortsByFullName.find(port_name);
    if (it != metaPortsByFullName.end()) {
        return it->second;
    } else {
        return 0;
    }
}

meta_jack_port_t * MetaJackContext::get_port_by_id(jack_port_id_t port_id)
{
    std::map<jack_port_id_t, _meta_jack_port*>::iterator it = metaPortsById.find(port_id);
    if (it != metaPortsById.end()) {
        return it->second;
    } else {
        return 0;
    }
}

jack_nframes_t MetaJackContext::get_frames_since_cycle_start()
{
    assert(active);
    return jack_frames_since_cycle_start(client);
}

jack_nframes_t MetaJackContext::get_frame_time()
{
    assert(active);
    return jack_frame_time(client);
}

jack_nframes_t MetaJackContext::get_last_frame_time()
{
    assert(active);
    return jack_last_frame_time(client);
}

jack_time_t MetaJackContext::convert_frames_to_time(jack_nframes_t nframes)
{
    assert(active);
    return jack_frames_to_time(client, nframes);
}

jack_nframes_t MetaJackContext::convert_time_to_frames(jack_time_t time)
{
    assert(active);
    return jack_time_to_frames(client, time);
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
    head->midiDataSize = head->midiEventCount = 0;
}

size_t MetaJackContext::midi_max_event_size(void* port_buffer)
{
    MetaJackContextMidiBufferHead *head = (MetaJackContextMidiBufferHead*)port_buffer;
    return head->bufferSize - head->midiEventCount * sizeof(jack_midi_data_t) - head->midiDataSize;
}

jack_midi_data_t* MetaJackContext::midi_event_reserve(void *port_buffer, jack_nframes_t  time, size_t data_size)
{
    MetaJackContextMidiBufferHead *head = (MetaJackContextMidiBufferHead*)port_buffer;
    char *charBuffer = (char*)port_buffer + sizeof(MetaJackContextMidiBufferHead);;
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
    // TODO
    return 0;
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

int MetaJackContext::process(_meta_jack_client *metaClient, std::set<_meta_jack_client*> &unprocessedClients, jack_nframes_t nframes)
{
    // recursively process all clients that are connected to this client's input ports first:
    for (std::set<_meta_jack_port*>::iterator i = metaClient->ports.begin(); i != metaClient->ports.begin(); i++) {
        _meta_jack_port *port = *i;
        if (port->isInput()) {
            port_get_buffer(port, nframes);
            // clear the port buffer:
            port->clearBuffer();
            for (std::set<_meta_jack_port*>::iterator j = port->connectedPorts.begin(); j != port->connectedPorts.end(); j++) {
                _meta_jack_port *connectedPort = *j;
                if (unprocessedClients.find(connectedPort->client) != unprocessedClients.end()) {
                    if (process(connectedPort->client, unprocessedClients, nframes)) {
                        return 1;
                    }
                }
                // add all buffers connected to input ports to this client's ports:
                connectedPort->mergeBuffers(port);
            }
        }
    }
    // process this client:
    if (metaClient->processCallback && metaClient->processCallback(nframes, metaClient->processCallbackArgument)) {
        return 1;
    }
    unprocessedClients.erase(metaClient);
    return 0;
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
        } else if (event.type == MetaJackContextEvent::UNREGISTER_PORT) {
            for (std::set<_meta_jack_port*>::iterator i = event.source->connectedPorts.begin(); i != event.source->connectedPorts.end(); i++) {
                _meta_jack_port *connectedPort = *i;
                event.source->disconnect(connectedPort);
                if (connectedPort->connectedPorts.size() == 0) {
                    connectedPort->client->ports.erase(connectedPort);
                    connectedPort->client->portsByShortName.erase(connectedPort->short_name);
                }
            }
            event.source->client->ports.erase(event.source);
            event.source->client->portsByShortName.erase(event.source->short_name);
        } else if (event.type == MetaJackContextEvent::CONNECT_PORTS) {
            event.source->client->ports.insert(event.source);
            event.source->client->portsByShortName[event.source->short_name] = event.source;
            event.dest->client->ports.insert(event.dest);
            event.dest->client->portsByShortName[event.dest->short_name] = event.dest;
            event.source->connect(event.dest);
        } else if (event.type == MetaJackContextEvent::DISCONNECT_PORTS) {
            event.source->disconnect(event.dest);
            if (event.source->connectedPorts.size() == 0) {
                event.source->client->ports.erase(event.source);
                event.source->client->portsByShortName.erase(event.source->short_name);
            }
            if (event.dest->connectedPorts.size() == 0) {
                event.dest->client->ports.erase(event.dest);
                event.dest->client->portsByShortName.erase(event.dest->short_name);
            }
        }
    }
    // wake the other thread(s):
    waitCondition.wakeAll();
    // TODO: evaluate the graph structure and call all process callbacks registered by internal clients
    std::set<_meta_jack_client*> unprocessedClients = metaClientsForProcess;
    bool failure = false;
    for (; !failure && unprocessedClients.size(); ) {
        // get an arbitrary client and process it:
        failure = process(*unprocessedClients.begin(), unprocessedClients, nframes);
    }
    return (failure ? 1 : 0);
}

int MetaJackContext::process(jack_nframes_t nframes, void *arg)
{
    return ((MetaJackContext*)arg)->process(nframes);
}

int MetaJackContext::processDummyInputClient(jack_nframes_t nframes, void *arg)
{
    MetaJackContext *context = (MetaJackContext*)arg;
    // copy from the real inputs to the dummy client outputs:
    _meta_jack_client *dummy = context->dummyInputClient;
    _meta_jack_port *audioOutputPort = dummy->portsByShortName["capture_audio"];
    _meta_jack_port *midiOutputPort = dummy->portsByShortName["capture_midi"];
    // copy audio:
    jack_default_audio_sample_t *audioOutputBuffer = (jack_default_audio_sample_t*)context->port_get_buffer(audioOutputPort, nframes);
    jack_default_audio_sample_t *audioInputBuffer = (jack_default_audio_sample_t*)jack_port_get_buffer(context->audioIn, nframes);
    for (jack_nframes_t i = 0; i < nframes; i++) {
        audioOutputBuffer[i] = audioInputBuffer[i];
    }
    // copy midi:
    void *midiOutputBuffer = context->port_get_buffer(midiOutputPort, nframes);
    void *midiInputBuffer = jack_port_get_buffer(context->midiIn, nframes);
    context->midi_clear_buffer(midiOutputBuffer);
    for (jack_nframes_t i = 0; i < jack_midi_get_event_count(midiInputBuffer); i++) {
        jack_midi_event_t event;
        jack_midi_event_get(&event, midiInputBuffer, i);
        context->midi_event_write(midiOutputBuffer, event.time, event.buffer, event.size);
    }
    return 0;
}

int MetaJackContext::processDummyOutputClient(jack_nframes_t nframes, void *arg)
{
    MetaJackContext *context = (MetaJackContext*)arg;
    // copy from the real inputs to the dummy client outputs:
    _meta_jack_client *dummy = context->dummyInputClient;
    _meta_jack_port *audioInputPort = dummy->portsByShortName["playback_audio"];
    _meta_jack_port *midiInputPort = dummy->portsByShortName["playback_midi"];
    // copy audio:
    jack_default_audio_sample_t *audioInputBuffer = (jack_default_audio_sample_t*)context->port_get_buffer(audioInputPort, nframes);
    jack_default_audio_sample_t *audioOutputBuffer = (jack_default_audio_sample_t*)jack_port_get_buffer(context->audioOut, nframes);
    for (jack_nframes_t i = 0; i < nframes; i++) {
        audioOutputBuffer[i] = audioInputBuffer[i];
    }
    // copy midi:
    void *midiInputBuffer = context->port_get_buffer(midiInputPort, nframes);
    void *midiOutputBuffer = jack_port_get_buffer(context->midiOut, nframes);
    jack_midi_clear_buffer(midiOutputBuffer);
    for (jack_nframes_t i = 0; i < jack_midi_get_event_count(midiInputBuffer); i++) {
        jack_midi_event_t event;
        context->midi_event_get(&event, midiInputBuffer, i);
        jack_midi_event_write(midiOutputBuffer, event.time, event.buffer, event.size);
    }
    return 0;
}
