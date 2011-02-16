#include "metajackcontext.h"
#include <sstream>
#include <cassert>
#include <list>
#include <boost/xpressive/xpressive_dynamic.hpp>

MetaJackContext * MetaJackContext::instance = &MetaJackContext::instance_;
MetaJackContext MetaJackContext::instance_("meta_jack");

MetaJackContext::MetaJackContext(const std::string &name) :
    wrapperClient(0),
    uniquePortId(1),
    graphChangesRingBuffer(1024),
    shutdown(false)
{
    // register at the real JACK server:
    wrapperClient = jack_client_open(name.c_str(), JackNullOption, 0);
    if (wrapperClient) {
        // get the buffer size:
        bufferSize = jack_get_buffer_size(wrapperClient);
        // register the process callback (this gets special treatment):
        jack_set_process_callback(wrapperClient, process, this);
        // register the thread init callback:
        jack_set_thread_init_callback(wrapperClient, JackThreadInitCallbackHandler::invokeCallbacksWithoutArgs, &threadInitCallbackHandler);
        // register the shutdown info callback (this gets special treatment):
        jack_on_info_shutdown(wrapperClient, infoShutdownCallback, this);
        // register the freewheel callback:
        jack_set_freewheel_callback(wrapperClient, JackFreewheelCallbackHandler::invokeCallbacksWithArgs, &freewheelCallbackHandler);
        // set the buffer size callback to be informed of any buffer size changed (this gets special treatment):
        jack_set_buffer_size_callback(wrapperClient, bufferSizeCallback, this);
        // register the sample rate callback:
        jack_set_sample_rate_callback(wrapperClient, JackSampleRateCallbackHandler::invokeCallbacksWithArgs, &sampleRateCallbackHandler);
        // register the xrun callback:
        jack_set_xrun_callback(wrapperClient, JackXRunCallbackHandler::invokeCallbacksWithoutArgs, &xRunCallbackHandler);
        // activate the client:
        if (jack_activate(wrapperClient)) {
            jack_client_close(wrapperClient);
            wrapperClient = 0;
        } else {
            MetaJackInterfaceClient *dummyInputClient = new MetaJackInterfaceClient(this, JackPortIsOutput);
            activateClient(dummyInputClient);
            MetaJackInterfaceClient *dummyOutputClient = new MetaJackInterfaceClient(this, JackPortIsInput);
            activateClient(dummyOutputClient);
        }
    }
}

MetaJackContext::~MetaJackContext()
{
    // invoke the internal clients' shutdown callbacks:
    // (note: no status is set, because there is no one that really fits this situation)
    infoShutdownCallbackHandler.invokeCallbacksWithArgs((jack_status_t)0, "Your MetaJack instance is being deleted");
    shutdownCallbackHandler.invokeCallbacks();
    // close the wrapper client:
    jack_client_close(wrapperClient);
    wrapperClient = 0;
    // close all clients:
    for (; clients.size(); ) {
        MetaJackClient *client = clients.begin()->second;
        closeClient(client);
    }
}

jack_port_t * MetaJackContext::createWrapperPort(const std::string &shortName, const std::string &type, unsigned long flags)
{
    return jack_port_register(wrapperClient, shortName.c_str(), type.c_str(), flags, 0);
}

bool MetaJackContext::isActive() const
{
    return wrapperClient && !shutdown;
}

size_t MetaJackContext::getClientNameSize()
{
    return jack_client_name_size();
}

size_t MetaJackContext::getPortNameSize()
{
    return jack_port_name_size();
}

size_t MetaJackContext::getPortTypeSize()
{
    return jack_port_type_size();
}

jack_port_id_t MetaJackContext::createUniquePortId()
{
    return uniquePortId++;
}

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
    bool nameIsTaken = (clients.find(name) != clients.end());
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
        nameIsTaken = (clients.find(clientName) != clients.end());
    }
    MetaJackClient *client = new MetaJackClient(clientName);
    clients[clientName] = client;
    clientRegistrationCallbackHandler.invokeCallbacksWithArgs(clientName.c_str(), 1);
    return client;
}

bool MetaJackContext::closeClient(MetaJackClient *client)
{
    assert(client && client->getProcessClient());
    deactivateClient(client);
    if (isActive()) {
        MetaJackGraphEvent event;
        event.type = MetaJackGraphEvent::CLOSE_CLIENT;
        event.client = client->getProcessClient();
        // the following will call the process thread's closeClient() method:
        sendGraphChangeEvent(event);
    } else {
        closeClient(client->getProcessClient());
    }
    clientRegistrationCallbackHandler.invokeCallbacksWithArgs(client->getName().c_str(), 0);
    clients.erase(client->getName());
    delete client;
    return true;
}

void MetaJackContext::closeClient(MetaJackClientProcess *client)
{
    assert(activeClients.find(client) == activeClients.end());
    delete client;
}

bool MetaJackContext::setProcessCallback(MetaJackClient *client, JackProcessCallback processCallback, void *processCallbackArgument)
{
    assert(client);
    if (client->isActive()) {
        return false;
    }
    if (isActive()) {
        MetaJackGraphEvent event;
        event.type = MetaJackGraphEvent::SET_PROCESS_CALLBACK;
        event.client = client->getProcessClient();
        event.processCallback = processCallback;
        event.processCallbackArgument = processCallbackArgument;
        // the following will call the process thread's setProcessCallback() method:
        sendGraphChangeEvent(event);
    } else {
        setProcessCallback(client->getProcessClient(), processCallback, processCallbackArgument);
    }
    return true;
}

void MetaJackContext::setProcessCallback(MetaJackClientProcess *client, JackProcessCallback processCallback, void *processCallbackArgument)
{
    assert(client);
    client->setProcessCallback(processCallback, processCallbackArgument);
}

bool MetaJackContext::activateClient(MetaJackClient *client)
{
    assert(client);
    if (client->isActive()) {
        return false;
    }
    // activate that client's buffer size callback, if it has any:
    std::map<MetaJackClient*, std::pair<JackBufferSizeCallback, void*> >::iterator find = bufferSizeCallbackHandler.find(client);
    if (find != bufferSizeCallbackHandler.end()) {
        JackBufferSizeCallback callback = find->second.first;
        void *arg = find->second.second;
        callback(bufferSize, arg);
    }
    if (isActive()) {
        MetaJackGraphEvent event;
        event.type = MetaJackGraphEvent::ACTIVATE_CLIENT;
        event.client = client->getProcessClient();
        // the following will call the process thread's activateClient() method:
        sendGraphChangeEvent(event);
    } else {
        activateClient(client->getProcessClient());
    }
    client->setActive(true);
    return true;
}

void MetaJackContext::activateClient(MetaJackClientProcess *client)
{
    assert(client);
    activeClients.insert(client);
}

bool MetaJackContext::deactivateClient(MetaJackClient *client)
{
    assert(client);
    if (!client->isActive()) {
        return false;
    }
    if (isActive()) {
        MetaJackGraphEvent event;
        event.type = MetaJackGraphEvent::DEACTIVATE_CLIENT;
        event.client = client->getProcessClient();
        // the following will call the process thread's deactivateClient() method:
        sendGraphChangeEvent(event);
    } else {
        deactivateClient(client->getProcessClient());
    }
    client->setActive(false);
    return true;
}

void MetaJackContext::deactivateClient(MetaJackClientProcess *client)
{
    assert(client);
    activeClients.erase(client);
    client->disconnect();
}

MetaJackPort * MetaJackContext::registerPort(MetaJackClient *client, const std::string & shortName, const std::string &type, unsigned long flags, unsigned long)
{
    assert(client);
    // check if the given port name is not too long:
    if (client->getName().length() + shortName.length() + 1 > getClientNameSize()) {
        return 0;
    }
    // check if the given port name is already taken:
    std::stringstream fullNameStream;
    fullNameStream << client->getName() << ":" << shortName;
    std::string fullName = fullNameStream.str();
    if (clients.find(fullName) != clients.end()) {
        return 0;
    }
    // consider only audio and midi ports (to be able to ignore buffer_size for now):
    if ((type != JACK_DEFAULT_AUDIO_TYPE) && (type != JACK_DEFAULT_MIDI_TYPE)) {
        return 0;
    }
    // make sure that the port is either input or output:
    if (!(flags & JackPortIsInput) && !(flags & JackPortIsOutput)) {
        return 0;
    }
    MetaJackPort *port = new MetaJackPort(client, createUniquePortId(), shortName, type, flags);
    port->createProcessPort(bufferSize);
    if (isActive()) {
        MetaJackGraphEvent event;
        event.type = MetaJackGraphEvent::REGISTER_PORT;
        event.client = client->getProcessClient();
        event.port = port->getProcessPort();
        event.nonProcessPort = port;
        // the following will call the process thread's registerPort() method:
        sendGraphChangeEvent(event);
    } else {
        registerPort(client->getProcessClient(), port->getProcessPort(), port);
    }
    portsById[port->getId()] = portsByName[port->getFullName()] = port;
    portRegistrationCallbackHandler.invokeCallbacksWithArgs(port->getId(), 1);
    return port;
}

void MetaJackContext::registerPort(MetaJackClientProcess *client, MetaJackPortProcess *port, MetaJackPort *nonProcessPort)
{
    assert(client && port);
    port->setClient(client);
    processPorts[nonProcessPort] = port;
}

bool MetaJackContext::unregisterPort(MetaJackPort *port)
{
    assert(port && port->getProcessPort());
    if (isActive()) {
        MetaJackGraphEvent event;
        event.type = MetaJackGraphEvent::UNREGISTER_PORT;
        event.port = port->getProcessPort();
        event.nonProcessPort = port;
        // the following will call the process thread's unregisterPort() method:
        sendGraphChangeEvent(event);
    } else {
        unregisterPort(port->getProcessPort(), port);
    }
    portRegistrationCallbackHandler.invokeCallbacksWithArgs(port->getId(), 0);
    portsById.erase(port->getId());
    portsByName.erase(port->getFullName());
    delete port;
    return true;
}

void MetaJackContext::unregisterPort(MetaJackPortProcess *port, MetaJackPort *nonProcessPort)
{
    assert(port);
    processPorts.erase(nonProcessPort);
    delete port;
}

bool MetaJackContext::renamePort(MetaJackPort *port, const std::string &shortName)
{
    assert(port && port->getProcessPort());
    std::string oldShortName = port->getShortName();
    std::string oldFullName = port->getFullName();
    port->setShortName(shortName);
    // check if the given port name is not too long and if the given port name is already taken:
    if ((port->getFullName().length() > getClientNameSize()) || (clients.find(port->getFullName()) != clients.end())) {
        port->setShortName(oldShortName);
        return false;
    }
    if (isActive()) {
        MetaJackGraphEvent event;
        event.type = MetaJackGraphEvent::RENAME_PORT;
        event.port = port->getProcessPort();
        event.shortName = shortName;
        // the following will call the process thread's renamePort() method:
        sendGraphChangeEvent(event);
    } else {
        renamePort(port->getProcessPort(), shortName);
    }
    portRenameCallbackHandler.invokeCallbacksWithArgs(port->getId(), oldFullName.c_str(), port->getFullName().c_str());
    portsByName.erase(oldFullName);
    portsByName[port->getFullName()] = port;
    return true;
}

void MetaJackContext::renamePort(MetaJackPortProcess *port, const std::string &shortName)
{
    assert(port);
    port->setShortName(shortName);
}

bool MetaJackContext::connectPorts(const std::string &sourceName, const std::string &destinationName)
{
    if (!isActive()) {
        return false;
    }
    MetaJackPort *source = getPortByName(sourceName);
    MetaJackPort *dest = getPortByName(destinationName);
    if (!source || !dest) {
        // ports could not be found by name
        return false;
    }
    if (source->isInput() == dest->isInput()) {
        // both ports are inputs or both are outputs
        return false;
    }
    if ((source->isInput() && source->isIndirectInputOf(dest)) || (dest->isInput() && dest->isIndirectInputOf(source))) {
        // the connection would create a cycle
        return false;
    }
    source->connect(dest);
    if (isActive()) {
        MetaJackGraphEvent event;
        event.type = MetaJackGraphEvent::CONNECT_PORTS;
        event.port = source->getProcessPort();
        event.connectedPort = dest->getProcessPort();
        // the following will call the process thread's connectPorts() method:
        sendGraphChangeEvent(event);
    } else {
        connectPorts(source->getProcessPort(), dest->getProcessPort());
    }
    portConnectCallbackHandler.invokeCallbacksWithArgs(source->getId(), dest->getId(), 1);
    return true;
}

void MetaJackContext::connectPorts(MetaJackPortProcess *source, MetaJackPortProcess *dest)
{
    assert(source && dest);
    source->connect(dest);
}

bool MetaJackContext::disconnectPorts(const std::string &sourceName, const std::string &destinationName)
{
    if (!isActive()) {
        return false;
    }
    MetaJackPort *source = getPortByName(sourceName);
    MetaJackPort *dest = getPortByName(destinationName);
    if (!source || !dest) {
        // ports could not be found by name
        return false;
    }
    if (!source->isConnectedTo(dest)) {
        // ports are not connected:
        return false;
    }
    source->disconnect(dest);
    if (isActive()) {
        MetaJackGraphEvent event;
        event.type = MetaJackGraphEvent::DISCONNECT_PORTS;
        event.port = source->getProcessPort();
        event.connectedPort = dest->getProcessPort();
        // the following will call the process thread's disconnectPorts() method:
        sendGraphChangeEvent(event);
    } else {
        disconnectPorts(source->getProcessPort(), dest->getProcessPort());
    }
    portConnectCallbackHandler.invokeCallbacksWithArgs(source->getId(), dest->getId(), 0);
    return true;
}

void MetaJackContext::disconnectPorts(MetaJackPortProcess *source, MetaJackPortProcess *dest)
{
    assert(source && dest);
    source->disconnect(dest);
}

const char ** MetaJackContext::getPortsByPattern(const std::string &port_name_pattern, const std::string &type_name_pattern, unsigned long flags)
{
    boost::xpressive::sregex regexPortNames = boost::xpressive::sregex::compile(port_name_pattern);
    boost::xpressive::sregex regexTypeNames = boost::xpressive::sregex::compile(type_name_pattern);
    std::list<MetaJackPort*> matchingPorts;
    for (std::map<std::string, MetaJackPort*>::iterator i = portsByName.begin(); i != portsByName.end(); i++) {
        MetaJackPort *port = i->second;
        if (((port->getFlags() & flags) == flags) && ((port_name_pattern.length() == 0) || boost::xpressive::regex_match(port->getFullName(), regexPortNames)) && ((type_name_pattern.length() == 0) || boost::xpressive::regex_match(port->getType(), regexTypeNames))) {
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

MetaJackPort * MetaJackContext::getPortByName(const std::string &name) const {
    std::map<std::string, MetaJackPort*>::const_iterator i = portsByName.find(name);
    if (i != portsByName.end()) {
        return i->second;
    } else {
        return 0;
    }
}

MetaJackPort * MetaJackContext::getPortById(jack_port_id_t id)
{
    std::map<jack_port_id_t, MetaJackPort*>::const_iterator i = portsById.find(id);
    if (i != portsById.end()) {
        return i->second;
    } else {
        return 0;
    }
}

void * MetaJackContext::getPortBuffer(MetaJackPort *port, jack_nframes_t nframes)
{
    std::map<MetaJackPort*, MetaJackPortProcess*>::iterator find = processPorts.find(port);
    if (find != processPorts.end()) {
        MetaJackPortProcess *processPort = find->second;
        return processPort->getBuffer(nframes);
    } else {
        return 0;
    }
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
        MetaJackContextMidiBufferHead *head = (MetaJackContextMidiBufferHead*)port_buffer;
        head->lostMidiEvents++;
        return 1;
    }
}

jack_nframes_t MetaJackContext::midi_get_lost_event_count(void *port_buffer)
{
    MetaJackContextMidiBufferHead *head = (MetaJackContextMidiBufferHead*)port_buffer;
    return head->lostMidiEvents;
}

bool MetaJackContext::compare_midi_events(const jack_midi_event_t &event1, const jack_midi_event_t &event2) {
    return event1.time < event2.time;
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

void MetaJackContext::sendGraphChangeEvent(const MetaJackGraphEvent &event)
{
    // write the event to the ring buffer:
    graphChangesRingBuffer.write(event);
}

int MetaJackContext::process(jack_nframes_t nframes)
{
    // first get all changes to the graph since the last call:
    for (; graphChangesRingBuffer.readSpace(); ) {
        MetaJackGraphEvent event = graphChangesRingBuffer.read();
        if (event.type == MetaJackGraphEvent::CLOSE_CLIENT) {
            closeClient(event.client);
        } else if (event.type == MetaJackGraphEvent::SET_PROCESS_CALLBACK) {
            setProcessCallback(event.client, event.processCallback, event.processCallbackArgument);
        } else if (event.type == MetaJackGraphEvent::ACTIVATE_CLIENT) {
            activateClient(event.client);
        } else if (event.type == MetaJackGraphEvent::DEACTIVATE_CLIENT) {
            deactivateClient(event.client);
        } else if (event.type == MetaJackGraphEvent::REGISTER_PORT) {
            registerPort(event.client, event.port, event.nonProcessPort);
        } else if (event.type == MetaJackGraphEvent::UNREGISTER_PORT) {
            unregisterPort(event.port, event.nonProcessPort);
        } else if (event.type == MetaJackGraphEvent::RENAME_PORT) {
            renamePort(event.port, event.shortName);
        } else if (event.type == MetaJackGraphEvent::CONNECT_PORTS) {
            connectPorts(event.port, event.connectedPort);
        } else if (event.type == MetaJackGraphEvent::DISCONNECT_PORTS) {
            disconnectPorts(event.port, event.connectedPort);
        }
    }
    // evaluate the graph structure and call all process callbacks registered by internal clients:
    std::set<MetaJackClientProcess*> unprocessedClients = activeClients;
    bool success = true;
    for (; success && unprocessedClients.size(); ) {
        // get an arbitrary client and process it:
        MetaJackClientProcess *client = *unprocessedClients.begin();
        success = client->process(unprocessedClients, nframes);
    }
    return (success ? 0 : 1);
}

int MetaJackContext::process(jack_nframes_t nframes, void *arg)
{
    MetaJackContext *context = (MetaJackContext*)arg;
    return context->process(nframes);
}

void MetaJackContext::infoShutdownCallback(jack_status_t statusCode, const char* reason, void *arg)
{
    MetaJackContext *context = (MetaJackContext*)arg;
    // calling jack_client_close() from this callback is not allowed... remember that server is being shutdown for later invocation:
    context->shutdown = true;
    // notify the internal clients:
    // first notify the clients which registered a simple shutdown callback:
    context->shutdownCallbackHandler.invokeCallbacks();
    // now those with the more complex callback (both sets should be mutually exclusive):
    context->infoShutdownCallbackHandler.invokeCallbacksWithArgs(statusCode, reason);
}

int MetaJackContext::bufferSizeCallback(jack_nframes_t bufferSize, void *arg)
{
    MetaJackContext *context = (MetaJackContext*)arg;
    context->bufferSize = bufferSize;
    // change all ports' buffer sizes:
    for (std::map<jack_port_id_t, MetaJackPort*>::iterator i = context->portsById.begin(); i != context->portsById.end(); i++) {
        MetaJackPort *port = i->second;
        port->getProcessPort()->changeBufferSize(bufferSize);
    }
    // now invoke all callbacks registered by the internal clients:
    context->bufferSizeCallbackHandler.invokeCallbacksWithArgs(bufferSize);
    return 0;
}
