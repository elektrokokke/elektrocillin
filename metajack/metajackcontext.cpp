#include "metajackcontext.h"
#include <sstream>
#include <cassert>
#include <list>
#include <QRegExp>

MetaJackContextNew * MetaJackContextNew::instance = &MetaJackContextNew::instance_;
MetaJackContextNew MetaJackContextNew::instance_("meta_jack");

MetaJackContextNew::MetaJackContextNew(const std::string &name) :
    wrapperClient(0),
    uniquePortId(1),
    graphChangesRingBuffer(1024)
{
    // register at the real JACK server:
    wrapperClient = jack_client_open(name.c_str(), JackNullOption, 0);
    if (wrapperClient) {
        // get the buffer size:
        bufferSize = jack_get_buffer_size(wrapperClient);
        // set the buffer size callback to be informed of any buffer size changed:
        jack_set_buffer_size_callback(wrapperClient, bufferSizeCallback, this);
        // register the process callback:
        jack_set_process_callback(wrapperClient, process, this);
        // register some ports:
        jack_port_t *wrapperAudioInputPort = jack_port_register(wrapperClient, "Audio in", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
        jack_port_t *wrapperAudioOutputPort = jack_port_register(wrapperClient, "Audio out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
        jack_port_t *wrapperMidiInputPort = jack_port_register(wrapperClient, "Midi in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
        jack_port_t *wrapperMidiOutputPort = jack_port_register(wrapperClient, "Midi out", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
        // activate the client:
        if (jack_activate(wrapperClient)) {
            jack_client_close(wrapperClient);
            wrapperClient = 0;
        } else {
            MetaJackDummyInputClientNew *dummyInputClient = new MetaJackDummyInputClientNew(this, wrapperAudioInputPort, wrapperMidiInputPort);
            activateClient(dummyInputClient);
            MetaJackDummyOutputClientNew *dummyOutputClient = new MetaJackDummyOutputClientNew(this, wrapperAudioOutputPort, wrapperMidiOutputPort);
            activateClient(dummyOutputClient);
        }
    }
}

MetaJackContextNew::~MetaJackContextNew()
{
    // close all clients:
    for (; clients.size(); ) {
        MetaJackClientNew *client = clients.begin()->second;
        closeClient(client);
    }
    // close the wrapper client:
    jack_client_close(wrapperClient);
}

bool MetaJackContextNew::isActive() const
{
    return wrapperClient;
}

size_t MetaJackContextNew::getClientNameSize()
{
    return jack_client_name_size();
}

size_t MetaJackContextNew::getPortNameSize()
{
    return jack_port_name_size();
}

size_t MetaJackContextNew::getPortTypeSize()
{
    return jack_port_type_size();
}

jack_port_id_t MetaJackContextNew::createUniquePortId()
{
    return uniquePortId++;
}

MetaJackClientNew * MetaJackContextNew::openClient(const std::string &name, jack_options_t options)
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
    MetaJackClientNew *client = new MetaJackClientNew(clientName);
    clients[clientName] = client;
    clientRegistrationCallbackHandler.invokeCallbacksWithArgs(clientName.c_str(), 1);
    return client;
}

bool MetaJackContextNew::closeClient(MetaJackClientNew *client)
{
    assert(client && client->getProcessClient());
    deactivateClient(client);
    clientRegistrationCallbackHandler.invokeCallbacksWithArgs(client->getName().c_str(), 0);
    clients.erase(client->getName());
    delete client->getProcessClient();
    delete client;
    return true;
}

bool MetaJackContextNew::setProcessCallback(MetaJackClientNew *client, JackProcessCallback processCallback, void *processCallbackArgument)
{
    assert(client);
    if (client->isActive()) {
        return false;
    }
    MetaJackGraphEventNew event;
    event.type = MetaJackGraphEventNew::SET_PROCESS_CALLBACK;
    event.client = client->getProcessClient();
    event.processCallback = processCallback;
    event.processCallbackArgument = processCallbackArgument;
    // the following will call the process thread's setProcessCallback() method:
    sendGraphChangeEvent(event);
    return true;
}

void MetaJackContextNew::setProcessCallback(MetaJackClientProcess *client, JackProcessCallback processCallback, void *processCallbackArgument)
{
    assert(client);
    client->setProcessCallback(processCallback, processCallbackArgument);
}

bool MetaJackContextNew::activateClient(MetaJackClientNew *client)
{
    assert(client);
    if (client->isActive()) {
        return false;
    }
    // activate that client's buffer size callback, if it has any:
    std::map<MetaJackClientNew*, std::pair<JackBufferSizeCallback, void*> >::iterator find = bufferSizeCallbackHandler.find(client);
    if (find != bufferSizeCallbackHandler.end()) {
        JackBufferSizeCallback callback = find->second.first;
        void *arg = find->second.second;
        callback(bufferSize, arg);
    }
    MetaJackGraphEventNew event;
    event.type = MetaJackGraphEventNew::ACTIVATE_CLIENT;
    event.client = client->getProcessClient();
    // the following will call the process thread's activateClient() method:
    sendGraphChangeEvent(event);
    client->setActive(true);
    return true;
}

void MetaJackContextNew::activateClient(MetaJackClientProcess *client)
{
    assert(client);
    activeClients.insert(client);
}

bool MetaJackContextNew::deactivateClient(MetaJackClientNew *client)
{
    assert(client);
    if (!client->isActive()) {
        return false;
    }
    MetaJackGraphEventNew event;
    event.type = MetaJackGraphEventNew::DEACTIVATE_CLIENT;
    event.client = client->getProcessClient();
    // the following will call the process thread's deactivateClient() method:
    sendGraphChangeEvent(event);
    client->setActive(false);
    return true;
}

void MetaJackContextNew::deactivateClient(MetaJackClientProcess *client)
{
    assert(client);
    activeClients.erase(client);
    client->disconnect();
}

MetaJackPortNew * MetaJackContextNew::registerPort(MetaJackClientNew *client, const std::string & shortName, const std::string &type, unsigned long flags, unsigned long)
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
    MetaJackPortNew *port = new MetaJackPortNew(client, createUniquePortId(), shortName, type, flags);
    port->createProcessPort(bufferSize);
    MetaJackGraphEventNew event;
    event.type = MetaJackGraphEventNew::REGISTER_PORT;
    event.client = client->getProcessClient();
    event.port = port->getProcessPort();
    // the following will call the process thread's registerPort() method:
    sendGraphChangeEvent(event);
    portsById[port->getId()] = portsByName[port->getFullName()] = port;
    portRegistrationCallbackHandler.invokeCallbacksWithArgs(port->getId(), 1);
    return port;
}

void MetaJackContextNew::registerPort(MetaJackClientProcess *client, MetaJackPortProcess *port)
{
    assert(client && port);
    port->setClient(client);
}

bool MetaJackContextNew::unregisterPort(MetaJackPortNew *port)
{
    assert(port && port->getProcessPort());
    MetaJackGraphEventNew event;
    event.type = MetaJackGraphEventNew::UNREGISTER_PORT;
    event.port = port->getProcessPort();
    // the following will call the process thread's unregisterPort() method:
    sendGraphChangeEvent(event);
    portRegistrationCallbackHandler.invokeCallbacksWithArgs(port->getId(), 0);
    portsById.erase(port->getId());
    portsByName.erase(port->getFullName());
    delete port;
    return true;
}

void MetaJackContextNew::unregisterPort(MetaJackPortProcess *port)
{
    assert(port);
    delete port;
}

bool MetaJackContextNew::renamePort(MetaJackPortNew *port, const std::string &shortName)
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
    MetaJackGraphEventNew event;
    event.type = MetaJackGraphEventNew::RENAME_PORT;
    event.port = port->getProcessPort();
    event.shortName = shortName;
    // the following will call the process thread's renamePort() method:
    sendGraphChangeEvent(event);
    portRenameCallbackHandler.invokeCallbacksWithArgs(port->getId(), oldFullName.c_str(), port->getFullName().c_str());
    portsByName.erase(oldFullName);
    portsByName[port->getFullName()] = port;
    return true;
}

void MetaJackContextNew::renamePort(MetaJackPortProcess *port, const std::string &shortName)
{
    assert(port);
    port->setShortName(shortName);
}

bool MetaJackContextNew::connectPorts(const std::string &sourceName, const std::string &destinationName)
{
    if (!isActive()) {
        return false;
    }
    MetaJackPortNew *source = getPortByName(sourceName);
    MetaJackPortNew *dest = getPortByName(destinationName);
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
    MetaJackGraphEventNew event;
    event.type = MetaJackGraphEventNew::CONNECT_PORTS;
    event.port = source->getProcessPort();
    event.connectedPort = dest->getProcessPort();
    // the following will call the process thread's connectPorts() method:
    sendGraphChangeEvent(event);
    portConnectCallbackHandler.invokeCallbacksWithArgs(source->getId(), dest->getId(), 1);
    return true;
}

void MetaJackContextNew::connectPorts(MetaJackPortProcess *source, MetaJackPortProcess *dest)
{
    assert(source && dest);
    source->connect(dest);
}

bool MetaJackContextNew::disconnectPorts(const std::string &sourceName, const std::string &destinationName)
{
    if (!isActive()) {
        return false;
    }
    MetaJackPortNew *source = getPortByName(sourceName);
    MetaJackPortNew *dest = getPortByName(destinationName);
    if (!source || !dest) {
        // ports could not be found by name
        return false;
    }
    if (!source->isConnectedTo(dest)) {
        // ports are not connected:
        return false;
    }
    source->disconnect(dest);
    MetaJackGraphEventNew event;
    event.type = MetaJackGraphEventNew::DISCONNECT_PORTS;
    event.port = source->getProcessPort();
    event.connectedPort = dest->getProcessPort();
    // the following will call the process thread's disconnectPorts() method:
    sendGraphChangeEvent(event);
    portConnectCallbackHandler.invokeCallbacksWithArgs(source->getId(), dest->getId(), 0);
    return true;
}

void MetaJackContextNew::disconnectPorts(MetaJackPortProcess *source, MetaJackPortProcess *dest)
{
    assert(source && dest);
    source->disconnect(dest);
}

const char ** MetaJackContextNew::getPortsByPattern(const std::string &port_name_pattern, const std::string &type_name_pattern, unsigned long flags)
{
    QRegExp regExpPortNames(port_name_pattern.c_str());
    QRegExp regExpTypeNames(type_name_pattern.c_str());
    std::list<MetaJackPortNew*> matchingPorts;
    for (std::map<std::string, MetaJackPortNew*>::iterator i = portsByName.begin(); i != portsByName.end(); i++) {
        MetaJackPortNew *port = i->second;
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
        for (std::list<MetaJackPortNew*>::iterator i = matchingPorts.begin(); i != matchingPorts.end(); i++, index++) {
            MetaJackPortNew *port = *i;
            names[index] = new char[port->getFullName().length() + 1];
            memcpy(names[index], port->getFullName().c_str(), port->getFullName().length() + 1);
        }
        names[index] = 0;
        return (const char**)names;
    } else {
        return 0;
    }
}

MetaJackPortNew * MetaJackContextNew::getPortByName(const std::string &name) const {
    std::map<std::string, MetaJackPortNew*>::const_iterator i = portsByName.find(name);
    if (i != portsByName.end()) {
        return i->second;
    } else {
        return 0;
    }
}

MetaJackPortNew * MetaJackContextNew::getPortById(jack_port_id_t id)
{
    std::map<jack_port_id_t, MetaJackPortNew*>::const_iterator i = portsById.find(id);
    if (i != portsById.end()) {
        return i->second;
    } else {
        return 0;
    }
}

int MetaJackContextNew::get_pid()
{
    assert(isActive());
    return jack_get_client_pid(jack_get_client_name(wrapperClient));
}

pthread_t MetaJackContextNew::get_thread_id()
{
    assert(isActive());
    return jack_client_thread_id(wrapperClient);
}

bool MetaJackContextNew::is_realtime()
{
    assert(isActive());
    return jack_is_realtime(wrapperClient);
}

int MetaJackContextNew::set_freewheel(int onoff)
{
    assert(isActive());
    return jack_set_freewheel(wrapperClient, onoff);
}

int MetaJackContextNew::set_buffer_size(jack_nframes_t nframes)
{
    assert(isActive());
    return jack_set_buffer_size(wrapperClient, nframes);
}

jack_nframes_t MetaJackContextNew::get_sample_rate()
{
    assert(isActive());
    return jack_get_sample_rate(wrapperClient);
}

jack_nframes_t MetaJackContextNew::get_buffer_size()
{
    assert(isActive());
    return jack_get_buffer_size(wrapperClient);
}

float MetaJackContextNew::get_cpu_load()
{
    assert(isActive());
    return jack_cpu_load(wrapperClient);
}

jack_nframes_t MetaJackContextNew::get_frames_since_cycle_start() const
{
    assert(isActive());
    return jack_frames_since_cycle_start(wrapperClient);
}

jack_nframes_t MetaJackContextNew::get_frame_time() const
{
    assert(isActive());
    return jack_frame_time(wrapperClient);
}

jack_nframes_t MetaJackContextNew::get_last_frame_time() const
{
    assert(isActive());
    return jack_last_frame_time(wrapperClient);
}

jack_time_t MetaJackContextNew::convert_frames_to_time(jack_nframes_t nframes) const
{
    assert(isActive());
    return jack_frames_to_time(wrapperClient, nframes);
}

jack_nframes_t MetaJackContextNew::convert_time_to_frames(jack_time_t time) const
{
    assert(isActive());
    return jack_time_to_frames(wrapperClient, time);
}

jack_time_t MetaJackContextNew::get_time()
{
    return jack_get_time();
}

jack_nframes_t MetaJackContextNew::midi_get_event_count(void* port_buffer)
{
    MetaJackContextMidiBufferHead *head = (MetaJackContextMidiBufferHead*)port_buffer;
    return head->midiEventCount;
}

int MetaJackContextNew::midi_event_get(jack_midi_event_t *event, void *port_buffer, jack_nframes_t event_index)
{
    // skip the buffer head:
    char *charBuffer = (char*)port_buffer + sizeof(MetaJackContextMidiBufferHead);;
    charBuffer += sizeof(jack_midi_event_t) * event_index;
    memcpy(event, charBuffer, sizeof(jack_midi_event_t));
    return 0;
}

void MetaJackContextNew::midi_clear_buffer(void *port_buffer)
{
    MetaJackContextMidiBufferHead *head = (MetaJackContextMidiBufferHead*)port_buffer;
    head->midiDataSize = head->midiEventCount = head->lostMidiEvents = 0;
}

size_t MetaJackContextNew::midi_max_event_size(void* port_buffer)
{
    MetaJackContextMidiBufferHead *head = (MetaJackContextMidiBufferHead*)port_buffer;
    return head->bufferSize - head->midiEventCount * sizeof(jack_midi_data_t) - head->midiDataSize;
}

jack_midi_data_t* MetaJackContextNew::midi_event_reserve(void *port_buffer, jack_nframes_t  time, size_t data_size)
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

int MetaJackContextNew::midi_event_write(void *port_buffer, jack_nframes_t time, const jack_midi_data_t *data, size_t data_size)
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

jack_nframes_t MetaJackContextNew::midi_get_lost_event_count(void *port_buffer)
{
    MetaJackContextMidiBufferHead *head = (MetaJackContextMidiBufferHead*)port_buffer;
    return head->lostMidiEvents;
}

bool MetaJackContextNew::compare_midi_events(const jack_midi_event_t &event1, const jack_midi_event_t &event2) {
    return event1.time < event2.time;
}

void MetaJackContextNew::free(void* ptr)
{
    if (ptr) {
        char **names = (char**)ptr;
        for (size_t index = 0; names[index]; index++) {
            delete [] names[index];
        }
        delete [] names;
    }
}

void MetaJackContextNew::sendGraphChangeEvent(const MetaJackGraphEventNew &event)
{
    // write the event to the ring buffer:
    graphChangesRingBuffer.write(event);
    // wait for the process thread to process it:
    waitMutex.lock();
    waitCondition.wait(&waitMutex);
    waitMutex.unlock();
}

int MetaJackContextNew::process(jack_nframes_t nframes)
{
    // first get all changes to the graph since the last call:
    for (; graphChangesRingBuffer.readSpace(); ) {
        MetaJackGraphEventNew event = graphChangesRingBuffer.read();
        if (event.type == MetaJackGraphEventNew::SET_PROCESS_CALLBACK) {
            setProcessCallback(event.client, event.processCallback, event.processCallbackArgument);
        } else if (event.type == MetaJackGraphEventNew::ACTIVATE_CLIENT) {
            activateClient(event.client);
        } else if (event.type == MetaJackGraphEventNew::DEACTIVATE_CLIENT) {
            deactivateClient(event.client);
        } else if (event.type == MetaJackGraphEventNew::REGISTER_PORT) {
            registerPort(event.client, event.port);
        } else if (event.type == MetaJackGraphEventNew::UNREGISTER_PORT) {
            unregisterPort(event.port);
        } else if (event.type == MetaJackGraphEventNew::RENAME_PORT) {
            renamePort(event.port, event.shortName);
        } else if (event.type == MetaJackGraphEventNew::CONNECT_PORTS) {
            connectPorts(event.port, event.connectedPort);
        } else if (event.type == MetaJackGraphEventNew::DISCONNECT_PORTS) {
            disconnectPorts(event.port, event.connectedPort);
        }
    }
    // wake the other thread(s):
    waitCondition.wakeAll();
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

int MetaJackContextNew::process(jack_nframes_t nframes, void *arg)
{
    MetaJackContextNew *context = (MetaJackContextNew*)arg;
    return context->process(nframes);
}

int MetaJackContextNew::bufferSizeCallback(jack_nframes_t bufferSize, void *arg)
{
    MetaJackContextNew *context = (MetaJackContextNew*)arg;
    context->bufferSize = bufferSize;
    // change all ports' buffer sizes:
    for (std::map<jack_port_id_t, MetaJackPortNew*>::iterator i = context->portsById.begin(); i != context->portsById.end(); i++) {
        MetaJackPortNew *port = i->second;
        port->getProcessPort()->changeBufferSize(bufferSize);
    }
    // now invoke all callbacks registered by the internal clients:
    context->bufferSizeCallbackHandler.invokeCallbacksWithArgs(bufferSize);
    return 0;
}
