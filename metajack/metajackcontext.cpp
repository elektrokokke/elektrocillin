#include "metajackcontext.h"
#include <sstream>
#include <cassert>
#include <list>
#include <memory.h>
//#include <boost/xpressive/xpressive_dynamic.hpp>
#include <QRegExp>

MetaJackContext::MetaJackContext(JackContext *jackInterface_, const std::string &name, unsigned int oversampling_) :
    wrapperInterface(jackInterface_),
    wrapperClient(0),
    wrapperClientName(name),
    uniquePortId(1),
    graphChangesRingBuffer(1024),
    shutdown(false),
    oversampling(oversampling_)
{
    // register at the given jack interface:
    wrapperClient = wrapperInterface->client_open(name.c_str(), JackNullOption, 0);
    if (wrapperClient) {
        // get the buffer size:
        bufferSize = wrapperInterface->get_buffer_size(wrapperClient) * oversampling;
        // register the process callback (this gets special treatment):
        wrapperInterface->set_process_callback(wrapperClient, process, this);
        // register the thread init callback:
        wrapperInterface->set_thread_init_callback(wrapperClient, JackThreadInitCallbackHandler::invokeCallbacksWithoutArgs, &threadInitCallbackHandler);
        // register the shutdown info callback (this gets special treatment):
        wrapperInterface->on_info_shutdown(wrapperClient, infoShutdownCallback, this);
        // register the freewheel callback:
        wrapperInterface->set_freewheel_callback(wrapperClient, JackFreewheelCallbackHandler::invokeCallbacksWithArgs, &freewheelCallbackHandler);
        // set the buffer size callback to be informed of any buffer size changed (this gets special treatment):
        wrapperInterface->set_buffer_size_callback(wrapperClient, bufferSizeCallback, this);
        // register the sample rate callback:
        wrapperInterface->set_sample_rate_callback(wrapperClient, JackSampleRateCallbackHandler::invokeCallbacksWithArgs, &sampleRateCallbackHandler);
        // register the xrun callback:
        wrapperInterface->set_xrun_callback(wrapperClient, JackXRunCallbackHandler::invokeCallbacksWithoutArgs, &xRunCallbackHandler);
        // register the transport sync callback:
        wrapperInterface->set_sync_callback(wrapperClient, JackSyncCallbackHandler::invokeCallbacksWithArgs, &syncCallbackHandler);
        // activate the client:
        if (wrapperInterface->activate(wrapperClient)) {
            wrapperInterface->client_close(wrapperClient);
            wrapperClient = 0;
        } else {
            wrapperClientName = wrapperInterface->get_client_name(wrapperClient);
            inputInterfaceClient = new MetaJackInterfaceClient(this, wrapperInterface, JackPortIsOutput);
            clients[inputInterfaceClient->getName()] = inputInterfaceClient;
            activateClient(inputInterfaceClient);
            outputInterfaceClient = new MetaJackInterfaceClient(this, wrapperInterface, JackPortIsInput);
            clients[outputInterfaceClient->getName()] = outputInterfaceClient;
            activateClient(outputInterfaceClient);
        }
    }
    std::stringstream nameStream;
    nameStream << wrapperInterface->get_name() << "/" << wrapperClientName;
    contextName = nameStream.str();
}

MetaJackContext::~MetaJackContext()
{
    // invoke the internal clients' shutdown callbacks:
    // (note: no status is set, because there is no one that really fits this situation)
    infoShutdownCallbackHandler.invokeCallbacksWithArgs((jack_status_t)0, "Your MetaJack instance is being deleted");
    shutdownCallbackHandler.invokeCallbacks();
    // close all clients:
    for (; clients.size(); ) {
        MetaJackClient *client = clients.begin()->second;
        closeClient(client);
    }
    // close the wrapper client:
    wrapperInterface->client_close(wrapperClient);
    wrapperClient = 0;
}

jack_port_t * MetaJackContext::createWrapperPort(const std::string &shortName, const std::string &type, unsigned long flags)
{
    return wrapperInterface->port_register(wrapperClient, shortName.c_str(), type.c_str(), flags, 0);
}

jack_client_t * MetaJackContext::getWrapperClient()
{
    return wrapperClient;
}

const char * MetaJackContext::getWrapperClientName()
{
    return wrapperClientName.c_str();
}

JackContext * MetaJackContext::getWrapperInterface()
{
    return wrapperInterface;
}

bool MetaJackContext::hasWrapperPorts() const
{
    return inputInterfaceClient->hasWrapperPorts() || outputInterfaceClient->hasWrapperPorts();
}

unsigned int MetaJackContext::getOversampling() const
{
    return oversampling;
}

bool MetaJackContext::isActive() const
{
    return wrapperClient && !shutdown;
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
    if (name.length() > (size_t)client_name_size()) {
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
    return client;
}

bool MetaJackContext::closeClient(MetaJackClient *client)
{
    assert(client && client->getProcessClient());
    // remove the client from all callback handlers:
    threadInitCallbackHandler.setCallback(client, 0, 0);
    shutdownCallbackHandler.setCallback(client, 0, 0);
    infoShutdownCallbackHandler.setCallback(client, 0, 0);
    freewheelCallbackHandler.setCallback(client, 0, 0);
    bufferSizeCallbackHandler.setCallback(client, 0, 0);
    sampleRateCallbackHandler.setCallback(client, 0, 0);
    clientRegistrationCallbackHandler.setCallback(client, 0, 0);
    portRegistrationCallbackHandler.setCallback(client, 0, 0);
    portConnectCallbackHandler.setCallback(client, 0, 0);
    portRenameCallbackHandler.setCallback(client, 0, 0);
    graphOrderCallbackHandler.setCallback(client, 0, 0);
    xRunCallbackHandler.setCallback(client, 0, 0);
    // deactivate the client:
    deactivateClient(client);
    // unregister the client's ports:
    for (; client->getPorts().size(); ) {
        unregisterPort((MetaJackPort*)*client->getPorts().begin());
    }
    if (isActive()) {
        MetaJackGraphEvent event;
        event.type = MetaJackGraphEvent::CLOSE_CLIENT;
        event.client = client->getProcessClient();
        // the following will call the process thread's closeClient() method:
        sendGraphChangeEvent(event);
    } else {
        closeClient(client->getProcessClient());
    }
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
    clientRegistrationCallbackHandler.invokeCallbacksWithArgs(client->getName().c_str(), 1);
    // invoke port registration callback for each port:
    for (std::set<MetaJackPortBase*>::iterator i = client->getPorts().begin(); i != client->getPorts().end(); i++) {
        portRegistrationCallbackHandler.invokeCallbacksWithArgs((*i)->getId(), 1);
    }
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
        // this event has to be synchronous, thus we wait here for the event to be processed before returning:
        waitMutex.lock();
        waitCondition.wait(&waitMutex);
        waitMutex.unlock();
    } else {
        deactivateClient(client->getProcessClient());
    }
    client->setActive(false);
    // disconnect all ports:
    for (std::set<MetaJackPortBase*>::iterator i = client->getPorts().begin(); i != client->getPorts().end(); i++) {
        port_disconnect((jack_client_t*)client, (jack_port_t*)*i);
    }
    // invoke port registration callback for each port:
    for (std::set<MetaJackPortBase*>::iterator i = client->getPorts().begin(); i != client->getPorts().end(); i++) {
        portRegistrationCallbackHandler.invokeCallbacksWithArgs((*i)->getId(), 0);
    }
    clientRegistrationCallbackHandler.invokeCallbacksWithArgs(client->getName().c_str(), 0);
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
    if (client->getName().length() + shortName.length() + 1 > (size_t)client_name_size()) {
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
    if (client->isActive()) {
        portRegistrationCallbackHandler.invokeCallbacksWithArgs(port->getId(), 1);
    }
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
    port->disconnect();
    if (((MetaJackClient*)port->getClient())->isActive()) {
        portRegistrationCallbackHandler.invokeCallbacksWithArgs(port->getId(), 0);
    }
    portsById.erase(port->getId());
    portsByName.erase(port->getFullName());
    delete port;
    return true;
}

void MetaJackContext::unregisterPort(MetaJackPortProcess *port, MetaJackPort *nonProcessPort)
{
    assert(port);
    processPorts.erase(nonProcessPort);
    port->disconnect();
    delete port;
}

bool MetaJackContext::renamePort(MetaJackPort *port, const std::string &shortName)
{
    assert(port && port->getProcessPort());
    std::string oldShortName = port->getShortName();
    std::string oldFullName = port->getFullName();
    port->setShortName(shortName);
    // check if the given port name is not too long and if the given port name is already taken:
    if ((port->getFullName().length() > (size_t)client_name_size()) || (clients.find(port->getFullName()) != clients.end())) {
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
    if (!(source->getFlags() & JackPortIsOutput)) {
        // source port must be an output port:
        return false;
    }
    if (!(dest->getFlags() & JackPortIsInput)) {
        // destination port must be an input port:
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
    if (!(source->getFlags() & JackPortIsOutput)) {
        // source port must be an output port:
        return false;
    }
    if (!(dest->getFlags() & JackPortIsInput)) {
        // destination port must be an input port:
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
//    boost::xpressive::sregex regexPortNames = boost::xpressive::sregex::compile(port_name_pattern);
//    boost::xpressive::sregex regexTypeNames = boost::xpressive::sregex::compile(type_name_pattern);
    QRegExp regexPortNames(port_name_pattern.c_str());
    QRegExp regexTypeNames(type_name_pattern.c_str());
    std::list<MetaJackPort*> matchingPorts;
    for (std::map<jack_port_id_t, MetaJackPort*>::iterator i = portsById.begin(); i != portsById.end(); i++) {
        MetaJackPort *port = i->second;
//        if (((port->getFlags() & flags) == flags) && ((port_name_pattern.length() == 0) || boost::xpressive::regex_match(port->getFullName(), regexPortNames)) && ((type_name_pattern.length() == 0) || boost::xpressive::regex_match(port->getType(), regexTypeNames))) {
        if (((port->getFlags() & flags) == flags) && ((port_name_pattern.length() == 0) || regexPortNames.exactMatch(port->getFullName().c_str())) && ((type_name_pattern.length() == 0) || regexTypeNames.exactMatch(port->getType().c_str()))) {
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

void MetaJackContext::midi_init_buffer(void *port_buffer, size_t bufferSizeInBytes)
{
    MetaJackContextMidiBufferHead *head = (MetaJackContextMidiBufferHead*)port_buffer;
    head->magic = MetaJackContextMidiBufferHead::MAGIC;
    head->bufferSize = bufferSizeInBytes - sizeof(MetaJackContext::MetaJackContextMidiBufferHead);
    head->midiDataSize = head->midiEventCount = head->lostMidiEvents = 0;
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
            waitCondition.wakeAll();
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
    return context->process(nframes * context->oversampling);
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
    context->bufferSize = bufferSize * context->oversampling;
    // change all ports' buffer sizes:
    for (std::map<jack_port_id_t, MetaJackPort*>::iterator i = context->portsById.begin(); i != context->portsById.end(); i++) {
        MetaJackPort *port = i->second;
        port->getProcessPort()->changeBufferSize(context->bufferSize);
    }
    // now invoke all callbacks registered by the internal clients:
    context->bufferSizeCallbackHandler.invokeCallbacksWithArgs(context->bufferSize);
    return 0;
}

/******************************************
 *
 * Methods reimplemented from JackInterface
 *
 ******************************************/
jack_client_t * MetaJackContext::client_by_name(const char *client_name)
{
    std::map<std::string, MetaJackClient*>::iterator find = clients.find(client_name);
    if (find != clients.end()) {
        return (jack_client_t*)find->second;
    } else {
        return 0;
    }
}

std::list<jack_client_t*> MetaJackContext::get_clients()
{
    std::list<jack_client_t*> clientList;
    for (std::map<std::string, MetaJackClient*>::iterator i = clients.begin(); i != clients.end(); i++) {
        clientList.push_back((jack_client_t*)i->second);
    }
    return clientList;
}

const char * MetaJackContext::get_name() const
{
    return contextName.c_str();
}

void MetaJackContext::get_version(int *major_ptr, int *minor_ptr, int *micro_ptr, int *proto_ptr)
{
    wrapperInterface->get_version(major_ptr, minor_ptr, micro_ptr, proto_ptr);
}

const char * MetaJackContext::get_version_string()
{
    return wrapperInterface->get_version_string();
}

jack_client_t * MetaJackContext::client_open (const char *client_name, jack_options_t options, jack_status_t *, ...)
{
    return (jack_client_t*)openClient(client_name, options);
}

int MetaJackContext::client_close (jack_client_t *client)
{
    return (closeClient((MetaJackClient*)client) ? 0 : 1);
}

int MetaJackContext::client_name_size ()
{
    return wrapperInterface->client_name_size();
}

char * MetaJackContext::get_client_name (jack_client_t *client)
{
    return (char*)((MetaJackClient*)client)->getName().c_str();
}

int MetaJackContext::activate (jack_client_t *client)
{
    return (activateClient((MetaJackClient*)client) ? 0 : 1);
}

int MetaJackContext::deactivate (jack_client_t *client)
{
    return (deactivateClient((MetaJackClient*)client) ? 0 : 1);
}

int MetaJackContext::get_client_pid (const char *name)
{
    return wrapperInterface->get_client_pid(wrapperInterface->get_client_name(wrapperClient));
}

pthread_t MetaJackContext::client_thread_id (jack_client_t *client)
{
    return wrapperInterface->client_thread_id(wrapperClient);
}

int MetaJackContext::is_realtime (jack_client_t *client)
{
    return wrapperInterface->is_realtime(wrapperClient);
}

int MetaJackContext::set_thread_init_callback (jack_client_t *client, JackThreadInitCallback thread_init_callback, void *arg)
{
    threadInitCallbackHandler.setCallback((MetaJackClient*)client, thread_init_callback, arg);
    return 0;
}

void MetaJackContext::on_shutdown (jack_client_t *client, JackShutdownCallback shutdown_callback, void *arg)
{
    // if the client has an info shutdown, do not register this callback (it wouldn't be called anyway):
    if (shutdownCallbackHandler.find((MetaJackClient*)client) == shutdownCallbackHandler.end()) {
        shutdownCallbackHandler.setCallback((MetaJackClient*)client, shutdown_callback, arg);
    }
}

void MetaJackContext::on_info_shutdown (jack_client_t *client, JackInfoShutdownCallback shutdown_callback, void *arg)
{
    infoShutdownCallbackHandler.setCallback((MetaJackClient*)client, shutdown_callback, arg);
    // remove any existing "simple" shutdown callback for this client (it shouldn't be called if the client registers an info shutdown callback):
    shutdownCallbackHandler.erase((MetaJackClient*)client);
}

int MetaJackContext::set_process_callback (jack_client_t *client, JackProcessCallback process_callback, void *arg)
{
    return (setProcessCallback((MetaJackClient*)client, process_callback, arg) ? 0 : 1);
}

int MetaJackContext::set_freewheel_callback (jack_client_t *client, JackFreewheelCallback freewheel_callback, void *arg)
{
    freewheelCallbackHandler.setCallback((MetaJackClient*)client, freewheel_callback, arg);
    return 0;
}

int MetaJackContext::set_buffer_size_callback (jack_client_t *client, JackBufferSizeCallback bufsize_callback, void *arg)
{
    bufferSizeCallbackHandler.setCallback((MetaJackClient*)client, bufsize_callback, arg);
    return 0;
}

int MetaJackContext::set_sample_rate_callback (jack_client_t *client, JackSampleRateCallback srate_callback, void *arg)
{
    sampleRateCallbackHandler.setCallback((MetaJackClient*)client, srate_callback, arg);
    return 0;
}

int MetaJackContext::set_client_registration_callback (jack_client_t *client, JackClientRegistrationCallback registration_callback, void *arg)
{
    clientRegistrationCallbackHandler.setCallback((MetaJackClient*)client, registration_callback, arg);
    return 0;
}

int MetaJackContext::set_port_registration_callback (jack_client_t *client, JackPortRegistrationCallback registration_callback, void *arg)
{
    portRegistrationCallbackHandler.setCallback((MetaJackClient*)client, registration_callback, arg);
    return 0;
}

int MetaJackContext::set_port_connect_callback (jack_client_t *client, JackPortConnectCallback connect_callback, void *arg)
{
    portConnectCallbackHandler.setCallback((MetaJackClient*)client, connect_callback, arg);
    return 0;
}

int MetaJackContext::set_port_rename_callback (jack_client_t *client, JackPortRenameCallback rename_callback, void *arg)
{
    portRenameCallbackHandler.setCallback((MetaJackClient*)client, rename_callback, arg);
    return 0;
}

int MetaJackContext::set_graph_order_callback (jack_client_t *client, JackGraphOrderCallback graph_callback, void *arg)
{
    graphOrderCallbackHandler.setCallback((MetaJackClient*)client, graph_callback, arg);
    return 0;
}

int MetaJackContext::set_xrun_callback (jack_client_t *client, JackXRunCallback xrun_callback, void *arg)
{
    xRunCallbackHandler.setCallback((MetaJackClient*)client, xrun_callback, arg);
    return 0;
}

int MetaJackContext::set_freewheel(jack_client_t *client, int onoff)
{
    return wrapperInterface->set_freewheel(wrapperClient, onoff);
}

int MetaJackContext::set_buffer_size (jack_client_t *client, jack_nframes_t nframes)
{
    return wrapperInterface->set_buffer_size(wrapperClient, nframes / oversampling);
}

jack_nframes_t MetaJackContext::get_sample_rate (jack_client_t *client)
{
    return wrapperInterface->get_sample_rate(wrapperClient) * oversampling;
}

jack_nframes_t MetaJackContext::get_buffer_size (jack_client_t *client)
{
    return wrapperInterface->get_buffer_size(wrapperClient) * oversampling;
}

float MetaJackContext::cpu_load (jack_client_t *client)
{
    return wrapperInterface->cpu_load(wrapperClient);
}

jack_port_t * MetaJackContext::port_register (jack_client_t *client, const char *port_name, const char *port_type, unsigned long flags, unsigned long buffer_size)
{
    return (jack_port_t*)registerPort((MetaJackClient*)client, port_name, port_type, flags, buffer_size);
}

int MetaJackContext::port_unregister (jack_client_t *client, jack_port_t *port)
{
    return (unregisterPort((MetaJackPort*)port) ? 0 : 1);
}

void * MetaJackContext::port_get_buffer (jack_port_t *port, jack_nframes_t nframes)
{
    return getPortBuffer((MetaJackPort*)port, nframes);
}

const char * MetaJackContext::port_name (const jack_port_t *port)
{
    return ((MetaJackPort*)port)->getFullName().c_str();
}

const char * MetaJackContext::port_short_name (const jack_port_t *port)
{
    return ((MetaJackPort*)port)->getShortName().c_str();
}

int MetaJackContext::port_flags (const jack_port_t *port)
{
    return ((MetaJackPort*)port)->getFlags();
}

const char * MetaJackContext::port_type (const jack_port_t *port)
{
    return ((MetaJackPort*)port)->getType().c_str();
}

int MetaJackContext::port_is_mine (const jack_client_t *client, const jack_port_t *port)
{
    return ((MetaJackPort*)port)->belongsTo((MetaJackClient*)client);
}

int MetaJackContext::port_connected (const jack_port_t *port)
{
    return ((MetaJackPort*)port)->getConnectionCount();
}

int MetaJackContext::port_connected_to (const jack_port_t *port, const char *port_name)
{
    MetaJackPort *connectedPort = getPortByName(port_name);
    if (connectedPort) {
        return ((MetaJackPort*)port)->isConnectedTo(connectedPort);
    } else {
        return 0;
    }
}

const char ** MetaJackContext::port_get_connections (const jack_port_t *port)
{
    return ((MetaJackPort*)port)->getConnections();
}

const char ** MetaJackContext::port_get_all_connections (const jack_client_t *client, const jack_port_t *port)
{
    return ((MetaJackPort*)port)->getConnections();
}

jack_nframes_t MetaJackContext::port_get_latency (jack_port_t *)
{
   return 0;
}

jack_nframes_t MetaJackContext::port_get_total_latency (jack_client_t *, jack_port_t *)
{
    return 0;
}

void MetaJackContext::port_set_latency (jack_port_t *, jack_nframes_t)
{
}

int MetaJackContext::recompute_total_latency (jack_client_t *, jack_port_t *)
{
    return 1;
}

int MetaJackContext::recompute_total_latencies (jack_client_t *)
{
    return 1;
}

int MetaJackContext::port_set_name (jack_port_t *port, const char *port_name)
{
    return (renamePort((MetaJackPort*)port, port_name) ? 0 : 1);
}

int MetaJackContext::port_set_alias (jack_port_t *client, const char *alias)
{
    return 1;
}

int MetaJackContext::port_unset_alias (jack_port_t *client, const char *alias)
{
    return 1;
}

int MetaJackContext::port_get_aliases (const jack_port_t *client, char* const aliases[])
{
    return 0;
}

int MetaJackContext::port_request_monitor (jack_port_t *port, int onoff)
{
    return 1;
}

int MetaJackContext::port_request_monitor_by_name (jack_client_t *client, const char *port_name, int onoff)
{
    return 1;
}

int MetaJackContext::port_ensure_monitor (jack_port_t *port, int onoff)
{
    return 1;
}

int MetaJackContext::port_monitoring_input (jack_port_t *port)
{
    return 0;
}

int MetaJackContext::connect (jack_client_t *client, const char *source_port, const char *destination_port)
{
    return (connectPorts(source_port, destination_port) ? 0 : 1);
}

int MetaJackContext::disconnect (jack_client_t *client, const char *source_port, const char *destination_port)
{
    return (disconnectPorts(source_port, destination_port) ? 0 : 1);
}

int MetaJackContext::port_disconnect (jack_client_t *client, jack_port_t *port)
{
    // disconnect all connections of the given port:
    MetaJackPort *metaJackPort = (MetaJackPort*)port;
    for (; metaJackPort->getConnectedPorts().size(); ) {
        if (metaJackPort->isInput()) {
            disconnectPorts((*metaJackPort->getConnectedPorts().begin())->getFullName(), metaJackPort->getFullName());
        } else {
            disconnectPorts(metaJackPort->getFullName(), (*metaJackPort->getConnectedPorts().begin())->getFullName());
        }
    }
    return 1;
}

int MetaJackContext::port_name_size()
{
    return wrapperInterface->port_name_size();
}

int MetaJackContext::port_type_size()
{
    return wrapperInterface->port_type_size();
}

const char ** MetaJackContext::get_ports (jack_client_t *client, const char *port_name_pattern, const char *type_name_pattern, unsigned long flags)
{
    if (client) {
        std::string port_name_pattern_string = (port_name_pattern ? port_name_pattern : "");
        std::string type_name_pattern_string = (type_name_pattern ? type_name_pattern : "");
        return getPortsByPattern(port_name_pattern_string, type_name_pattern_string, flags);
    } else {
        return 0;
    }
}

jack_port_t * MetaJackContext::port_by_name (jack_client_t *client, const char *port_name)
{
    return (jack_port_t*)getPortByName(port_name);
}

jack_port_t * MetaJackContext::port_by_id (jack_client_t *client, jack_port_id_t port_id)
{
    return (jack_port_t*)getPortById(port_id);
}

jack_nframes_t MetaJackContext::frames_since_cycle_start (const jack_client_t *client)
{
    return wrapperInterface->frames_since_cycle_start(wrapperClient) * oversampling;
}

jack_nframes_t MetaJackContext::frame_time (const jack_client_t *client)
{
    return wrapperInterface->frame_time(wrapperClient) * oversampling;
}

jack_nframes_t MetaJackContext::last_frame_time (const jack_client_t *client)
{
    return wrapperInterface->last_frame_time(wrapperClient) * oversampling;
}

jack_time_t MetaJackContext::frames_to_time(const jack_client_t *client, jack_nframes_t nframes)
{
    return wrapperInterface->frames_to_time(wrapperClient, nframes / oversampling);
}

jack_nframes_t MetaJackContext::time_to_frames(const jack_client_t *client, jack_time_t time)
{
    return wrapperInterface->time_to_frames(wrapperClient, time) * oversampling;
}

jack_time_t MetaJackContext::get_time()
{
    return wrapperInterface->get_time();
}

void MetaJackContext::set_error_function (void (*func)(const char *))
{
    wrapperInterface->set_error_function(func);
}

void MetaJackContext::set_info_function (void (*func)(const char *))
{
    wrapperInterface->set_error_function(func);
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

int MetaJackContext::release_timebase (jack_client_t *client)
{
    return wrapperInterface->release_timebase(wrapperClient);
}

int MetaJackContext::set_sync_callback (jack_client_t *client, JackSyncCallback sync_callback, void *arg)
{
    syncCallbackHandler.setCallback((MetaJackClient*)client, sync_callback, arg);
    return 0;
}

int MetaJackContext::set_sync_timeout (jack_client_t *client, jack_time_t timeout)
{
    return wrapperInterface->set_sync_timeout(wrapperClient, timeout);
}

int MetaJackContext::set_timebase_callback (jack_client_t *client, int conditional, JackTimebaseCallback timebase_callback, void *arg)
{
    return wrapperInterface->set_timebase_callback(wrapperClient, conditional, timebase_callback, arg);
}

int MetaJackContext::transport_locate (jack_client_t *client, jack_nframes_t frame)
{
    return wrapperInterface->transport_locate(wrapperClient, frame);
}

jack_transport_state_t MetaJackContext::transport_query (const jack_client_t *client, jack_position_t *pos)
{
    return wrapperInterface->transport_query(wrapperClient, pos);
}

jack_nframes_t MetaJackContext::get_current_transport_frame (const jack_client_t *client)
{
    return wrapperInterface->get_current_transport_frame(wrapperClient);
}

int MetaJackContext::transport_reposition (jack_client_t *client, jack_position_t *pos)
{
    return wrapperInterface->transport_reposition(wrapperClient, pos);
}

void MetaJackContext::transport_start (jack_client_t *client)
{
    wrapperInterface->transport_start(wrapperClient);
}

void MetaJackContext::transport_stop (jack_client_t *client)
{
    wrapperInterface->transport_stop(wrapperClient);
}

void MetaJackContext::get_transport_info (jack_client_t *client, jack_transport_info_t *tinfo)
{
    wrapperInterface->get_transport_info(wrapperClient, tinfo);
}

void MetaJackContext::set_transport_info (jack_client_t *client, jack_transport_info_t *tinfo)
{
    wrapperInterface->set_transport_info(wrapperClient, tinfo);
}
