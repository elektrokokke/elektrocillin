#include "recursivejackcontext.h"
#include "realjackcontext.h"
#include "metajackcontext.h"
#include <QStringList>

RecursiveJackContext RecursiveJackContext::instance;

RecursiveJackContext * RecursiveJackContext::getInstance()
{
    return &instance;
}

RecursiveJackContext::RecursiveJackContext()
{
    // put the interface to the real server on the stack as the first interface to be used:
    interfaces.push(new RealJackContext());
    interfaceStack.push(interfaces.top());
    // put one meta jack instance on the stack:
    pushNewContext("metajack");
}

RecursiveJackContext::~RecursiveJackContext()
{
    // delete all jack interfaces:
    for (; interfaces.size(); ) {
        delete interfaces.top();
        interfaces.pop();
    }
}

JackContext * RecursiveJackContext::getCurrentContext()
{
    return interfaceStack.top();
}

JackContext * RecursiveJackContext::pushNewContext(const std::string &desiredWrapperClientName, unsigned int oversampling)
{
    // create a new meta jack context:
    MetaJackContext *context = new MetaJackContext(interfaceStack.top(), desiredWrapperClientName, oversampling);
    interfaces.push(context);
    // get its name in the current jack context and remember it:
    std::string wrapperClientName = context->getWrapperInterface()->get_client_name(context->getWrapperClient());
    mapClientToInterface[context->getWrapperClient()] = context;
    mapClientNameToInterface[context->getWrapperInterface()][wrapperClientName] = context;
    // it becomes the current jack interface:
    interfaceStack.push(context);
    return context;
}

JackContext * RecursiveJackContext::pushExistingContext(JackContext *jackInterface)
{
    interfaceStack.push(jackInterface);
    return jackInterface;
}

JackContext * RecursiveJackContext::pushExistingContextByClient(jack_client_t *client)
{
    // get the jack interface associated with the given client:
    JackContext *jackInterface = mapClientToInterface[client];
    // it becomes the current jack interfaces:
    interfaceStack.push(jackInterface);
    return jackInterface;
}

JackContext * RecursiveJackContext::popContext()
{
    if (interfaceStack.size() > 1) {
        // remove the current jack interface from the stack, the previous interface becomes the current:
        interfaceStack.pop();
    }
    return interfaceStack.top();
}

void RecursiveJackContext::deleteContext(JackContext *context)
{
    // remove the context from our list and stack:
    std::stack<JackContext*> temp;
    for (; interfaceStack.size(); ) {
        if (interfaceStack.top() != context) {
            temp.push(interfaceStack.top());
        }
        interfaceStack.pop();
    }
    for (; temp.size(); ) {
        interfaceStack.push(temp.top());
        temp.pop();
    }
    for (; interfaces.size(); ) {
        if (interfaces.top() != context) {
            temp.push(interfaces.top());
        }
        interfaces.pop();
    }
    for (; temp.size(); ) {
        interfaces.push(temp.top());
        temp.pop();
    }
    delete context;
}

size_t RecursiveJackContext::getContextStackSize() const
{
    return interfaceStack.size();
}

void RecursiveJackContext::saveCurrentContext(QDataStream &stream, JackClientSerializer *clientSaver)
{
    JackContext *context = getCurrentContext();
    // get all clients in the context:
    std::list<jack_client_t*> clients = context->get_clients();
    // save the client count:
    int clientCount = clients.size();
    stream << clientCount;
    // save their state:
    for (std::list<jack_client_t*>::iterator i = clients.begin(); i != clients.end(); i++) {
        jack_client_t *client = *i;
        // save the client name:
        QString clientName = context->get_client_name(client);
        stream << clientName;
        // save the client's property:
        QVariant property = clientProperties[client];
        stream << property;
        // determine if the current client is a wrapper client:
        JackContext *wrapperContext = getContextByClientName(context, clientName.toAscii().data());
        stream << (wrapperContext != 0);
        if (wrapperContext) {
            // save the context:
            pushExistingContext(wrapperContext);
            saveCurrentContext(stream, clientSaver);
            popContext();
        } else {
            // let the client saver save the client's state:
            clientSaver->save(client, stream);
        }
    }
    // save the connections:
    QStringList connections;
    if (clients.size()) {
        jack_client_t *client = clients.back();
        const char ** portNames = context->get_ports(client, 0, 0, JackPortIsOutput);
        if (portNames) {
            for (int i = 0; portNames[i]; i++) {
                jack_port_t *port = context->port_by_name(client, portNames[i]);
                const char **otherPortNames = context->port_get_all_connections(client, port);
                if (otherPortNames) {
                    for (int j = 0; otherPortNames[j]; j++) {
                        QString connection = QString("%1::%2").arg(portNames[i]).arg(otherPortNames[j]);
                        connections.append(connection);
                    }
                    // free the array:
                    context->free(otherPortNames);
                }
            }
            // free the array:
            context->free(portNames);
        }
    }
    stream << connections;
}

void RecursiveJackContext::loadCurrentContext(QDataStream &stream, JackClientSerializer *clientLoader)
{
    // load the client count:
    int clientCount;
    stream >> clientCount;
    // load their state:
    for (int i = 0; i < clientCount; i++) {
        // load the client name:
        QString clientName;
        stream >> clientName;
        // load the client's property:
        QVariant property;
        stream >> property;
        // determine if the client is a wrapper client:
        bool isWrapperClient;
        stream >> isWrapperClient;
        if (isWrapperClient) {
            // create a new context:
            MetaJackContext *wrapperContext = (MetaJackContext*)pushNewContext(clientName.toAscii().data());
            // load the context:
            loadCurrentContext(stream, clientLoader);
            popContext();
            jack_client_t *client = wrapperContext->getWrapperClient();
            clientName = get_client_name(client);
            setClientProperty(clientName, property);
        } else {
            // let the client loader load the client:
            jack_client_t *client = clientLoader->load(clientName, stream);
            if (client) {
                clientName = get_client_name(client);
                setClientProperty(clientName, property);
            } else {
                setClientProperty(clientName, property);
            }
        }
    }
    jack_client_t *client = getCurrentContext()->get_clients().back();
    // load the connections:
    QStringList connections;
    stream >> connections;
    for (int i = 0; i < connections.size(); i++) {
        QStringList portNames = connections[i].split("::");
        QString sourcePortName = portNames[0];
        QString destPortName = portNames[1];
        getCurrentContext()->connect(client, sourcePortName.toAscii().data(), destPortName.toAscii().data());
    }
}

void RecursiveJackContext::setClientProperty(const QString &clientName, QVariant property)
{
    setClientProperty(interfaceStack.top(), clientName, property);
}

QVariant RecursiveJackContext::getClientProperty(const QString &clientName)
{
    return getClientProperty(interfaceStack.top(), clientName);
}

void RecursiveJackContext::setClientProperty(JackContext *context, const QString &clientName, QVariant property)
{
    // get the client by the client's name in the given context:
    jack_client_t *client = context->client_by_name(clientName.toAscii().data());
    if (client) {
        setClientProperty(client, property);
    }
}

QVariant RecursiveJackContext::getClientProperty(JackContext *context, const QString &clientName)
{
    // get the client by the client's name in the given context:
    jack_client_t *client = context->client_by_name(clientName.toAscii().data());
    if (client) {
        return getClientProperty(client);
    } else {
        return QVariant();
    }
}

void RecursiveJackContext::setClientProperty(jack_client_t *client, QVariant property)
{
    clientProperties[client] = property;
}

QVariant RecursiveJackContext::getClientProperty(jack_client_t *client)
{
    return clientProperties[client];
}

JackContext * RecursiveJackContext::getContextByClientName(const std::string &clientName)
{
    return getContextByClientName(interfaceStack.top(), clientName);
}

JackContext * RecursiveJackContext::getContextByClientName(JackContext *context, const std::string &clientName)
{
    std::map<std::string, JackContext*>::iterator find = mapClientNameToInterface[context].find(clientName);
    if (find != mapClientNameToInterface[context].end()) {
        return find->second;
    } else {
        return 0;
    }
}

jack_client_t * RecursiveJackContext::client_by_name(const char *client_name)
{
    return interfaceStack.top()->client_by_name(client_name);
}

std::list<jack_client_t*> RecursiveJackContext::get_clients()
{
    return interfaceStack.top()->get_clients();
}

const char * RecursiveJackContext::get_name() const
{
    return interfaceStack.top()->get_name();
}

void RecursiveJackContext::get_version(int *major_ptr, int *minor_ptr, int *micro_ptr, int *proto_ptr)
{
    interfaceStack.top()->get_version(major_ptr, minor_ptr, micro_ptr, proto_ptr);
}

const char * RecursiveJackContext::get_version_string()
{
    return interfaceStack.top()->get_version_string();
}

jack_client_t * RecursiveJackContext::client_open (const char *client_name, jack_options_t options, jack_status_t *, ...)
{
    jack_client_t *client = interfaceStack.top()->client_open(client_name, options, 0);
    if (client) {
        mapClientToInterface[client] = interfaceStack.top();
    }
    return client;
}

int RecursiveJackContext::client_close (jack_client_t *client)
{
    JackContext *jackInterface = mapClientToInterface[client];
    std::string clientName = jackInterface->get_client_name(client);
    int returnValue = jackInterface->client_close(client);
    if (returnValue == 0) {
        mapClientToInterface.erase(client);
        mapClientNameToInterface[jackInterface].erase(clientName);
    }
    return returnValue;
}

int RecursiveJackContext::client_name_size ()
{
    return interfaceStack.top()->client_name_size();
}

char * RecursiveJackContext::get_client_name (jack_client_t *client)
{
    return mapClientToInterface[client]->get_client_name(client);
}

int RecursiveJackContext::activate (jack_client_t *client)
{
    return mapClientToInterface[client]->activate(client);
}

int RecursiveJackContext::deactivate (jack_client_t *client)
{
    return mapClientToInterface[client]->deactivate(client);
}

int RecursiveJackContext::get_client_pid (const char *name)
{
    return interfaceStack.top()->get_client_pid(name);
}

pthread_t RecursiveJackContext::client_thread_id (jack_client_t *client)
{
    return mapClientToInterface[client]->client_thread_id(client);
}

int RecursiveJackContext::is_realtime (jack_client_t *client)
{
    return mapClientToInterface[client]->is_realtime(client);
}

int RecursiveJackContext::set_thread_init_callback (jack_client_t *client, JackThreadInitCallback thread_init_callback, void *arg)
{
    return mapClientToInterface[client]->set_thread_init_callback(client, thread_init_callback, arg);
}

void RecursiveJackContext::on_shutdown (jack_client_t *client, JackShutdownCallback shutdown_callback, void *arg)
{
    mapClientToInterface[client]->on_shutdown(client, shutdown_callback, arg);
}

void RecursiveJackContext::on_info_shutdown (jack_client_t *client, JackInfoShutdownCallback shutdown_callback, void *arg)
{
    mapClientToInterface[client]->on_info_shutdown(client, shutdown_callback, arg);
}

int RecursiveJackContext::set_process_callback (jack_client_t *client, JackProcessCallback process_callback, void *arg)
{
    return mapClientToInterface[client]->set_process_callback(client, process_callback, arg);
}

int RecursiveJackContext::set_freewheel_callback (jack_client_t *client, JackFreewheelCallback freewheel_callback, void *arg)
{
    return mapClientToInterface[client]->set_freewheel_callback(client, freewheel_callback, arg);
}

int RecursiveJackContext::set_buffer_size_callback (jack_client_t *client, JackBufferSizeCallback bufsize_callback, void *arg)
{
    return mapClientToInterface[client]->set_buffer_size_callback(client, bufsize_callback, arg);
}

int RecursiveJackContext::set_sample_rate_callback (jack_client_t *client, JackSampleRateCallback srate_callback, void *arg)
{
    return mapClientToInterface[client]->set_sample_rate_callback(client, srate_callback, arg);
}

int RecursiveJackContext::set_client_registration_callback (jack_client_t *client, JackClientRegistrationCallback registration_callback, void *arg)
{
    return mapClientToInterface[client]->set_client_registration_callback(client, registration_callback, arg);
}

int RecursiveJackContext::set_port_registration_callback (jack_client_t *client, JackPortRegistrationCallback registration_callback, void *arg)
{
    return mapClientToInterface[client]->set_port_registration_callback(client, registration_callback, arg);
}

int RecursiveJackContext::set_port_connect_callback (jack_client_t *client, JackPortConnectCallback connect_callback, void *arg)
{
    return mapClientToInterface[client]->set_port_connect_callback(client, connect_callback, arg);
}

int RecursiveJackContext::set_port_rename_callback (jack_client_t *client, JackPortRenameCallback rename_callback, void *arg)
{
    return mapClientToInterface[client]->set_port_rename_callback(client, rename_callback, arg);
}

int RecursiveJackContext::set_graph_order_callback (jack_client_t *client, JackGraphOrderCallback graph_callback, void *arg)
{
    return mapClientToInterface[client]->set_graph_order_callback(client, graph_callback, arg);
}

int RecursiveJackContext::set_xrun_callback (jack_client_t *client, JackXRunCallback xrun_callback, void *arg)
{
    return mapClientToInterface[client]->set_xrun_callback(client, xrun_callback, arg);
}

int RecursiveJackContext::set_freewheel(jack_client_t *client, int onoff)
{
    return mapClientToInterface[client]->set_freewheel(client, onoff);
}

int RecursiveJackContext::set_buffer_size (jack_client_t *client, jack_nframes_t nframes)
{
    return mapClientToInterface[client]->set_buffer_size(client, nframes);
}

jack_nframes_t RecursiveJackContext::get_sample_rate (jack_client_t *client)
{
    return mapClientToInterface[client]->get_sample_rate(client);
}

jack_nframes_t RecursiveJackContext::get_buffer_size (jack_client_t *client)
{
    return mapClientToInterface[client]->get_buffer_size(client);
}

float RecursiveJackContext::cpu_load (jack_client_t *client)
{
    return mapClientToInterface[client]->cpu_load(client);
}

jack_port_t * RecursiveJackContext::port_register (jack_client_t *client, const char *port_name, const char *port_type, unsigned long flags, unsigned long buffer_size)
{
    jack_port_t *port = mapClientToInterface[client]->port_register(client, port_name, port_type, flags, buffer_size);
    if (port) {
        mapPortToInterface[port] = mapClientToInterface[client];
    }
    return port;
}

int RecursiveJackContext::port_unregister (jack_client_t *client, jack_port_t *port)
{
    int returnValue = mapClientToInterface[client]->port_unregister(client, port);
    if (returnValue == 0) {
        mapPortToInterface.erase(port);
    }
    return returnValue;
}

void * RecursiveJackContext::port_get_buffer (jack_port_t *port, jack_nframes_t nframes)
{
    return mapPortToInterface[port]->port_get_buffer(port, nframes);
}

const char * RecursiveJackContext::port_name (const jack_port_t *port)
{
    return mapPortToInterface[port]->port_name(port);
}

const char * RecursiveJackContext::port_short_name (const jack_port_t *port)
{
    return mapPortToInterface[port]->port_short_name(port);
}

int RecursiveJackContext::port_flags (const jack_port_t *port)
{
    return mapPortToInterface[port]->port_flags(port);
}

const char * RecursiveJackContext::port_type (const jack_port_t *port)
{
    return mapPortToInterface[port]->port_type(port);
}

int RecursiveJackContext::port_is_mine (const jack_client_t *client, const jack_port_t *port)
{
    return mapClientToInterface[client]->port_is_mine(client, port);
}

int RecursiveJackContext::port_connected (const jack_port_t *port)
{
    return mapPortToInterface[port]->port_connected(port);
}

int RecursiveJackContext::port_connected_to (const jack_port_t *port, const char *port_name)
{
    return mapPortToInterface[port]->port_connected_to(port, port_name);
}

const char ** RecursiveJackContext::port_get_connections (const jack_port_t *port)
{
    const char ** returnValue = mapPortToInterface[port]->port_get_connections(port);
    mapPointerToInterface[(void*)returnValue] = mapPortToInterface[port];
    return returnValue;
}

const char ** RecursiveJackContext::port_get_all_connections (const jack_client_t *client, const jack_port_t *port)
{
    const char ** returnValue = mapClientToInterface[client]->port_get_all_connections(client, port);
    mapPointerToInterface[(void*)returnValue] = mapClientToInterface[client];
    return returnValue;
}

jack_nframes_t RecursiveJackContext::port_get_latency (jack_port_t *port)
{
    return mapPortToInterface[port]->port_get_latency(port);
}

jack_nframes_t RecursiveJackContext::port_get_total_latency (jack_client_t *client, jack_port_t *port)
{
    return mapClientToInterface[client]->port_get_total_latency(client, port);
}

void RecursiveJackContext::port_set_latency (jack_port_t *port, jack_nframes_t nframes) {
    mapPortToInterface[port]->port_set_latency(port, nframes);
}

int RecursiveJackContext::recompute_total_latency (jack_client_t *client, jack_port_t *port)
{
    return mapClientToInterface[client]->recompute_total_latency(client, port);
}

int RecursiveJackContext::recompute_total_latencies (jack_client_t *client)
{
    return mapClientToInterface[client]->recompute_total_latencies(client);
}

int RecursiveJackContext::port_set_name (jack_port_t *port, const char *port_name)
{
    return mapPortToInterface[port]->port_set_name(port, port_name);
}

int RecursiveJackContext::port_set_alias (jack_port_t *port, const char *alias) {
    return mapPortToInterface[port]->port_set_alias(port, alias);
}

int RecursiveJackContext::port_unset_alias (jack_port_t *port, const char *alias)
{
    return mapPortToInterface[port]->port_unset_alias(port, alias);
}

int RecursiveJackContext::port_get_aliases (const jack_port_t *port, char* const aliases[])
{
    return mapPortToInterface[port]->port_get_aliases(port, aliases);
}

int RecursiveJackContext::port_request_monitor (jack_port_t *port, int onoff)
{
    return mapPortToInterface[port]->port_request_monitor(port, onoff);
}

int RecursiveJackContext::port_request_monitor_by_name (jack_client_t *client, const char *port_name, int onoff)
{
    return mapClientToInterface[client]->port_request_monitor_by_name(client, port_name, onoff);
}

int RecursiveJackContext::port_ensure_monitor (jack_port_t *port, int onoff)
{
    return mapPortToInterface[port]->port_ensure_monitor(port, onoff);
}

int RecursiveJackContext::port_monitoring_input (jack_port_t *port)
{
    return mapPortToInterface[port]->port_monitoring_input(port);
}

int RecursiveJackContext::connect (jack_client_t *client, const char *source_port, const char *destination_port)
{
    return mapClientToInterface[client]->connect(client, source_port, destination_port);
}

int RecursiveJackContext::disconnect (jack_client_t *client, const char *source_port, const char *destination_port)
{
    return mapClientToInterface[client]->disconnect(client, source_port, destination_port);
}

int RecursiveJackContext::port_disconnect (jack_client_t *client, jack_port_t *port)
{
    return mapClientToInterface[client]->port_disconnect(client, port);
}

int RecursiveJackContext::port_name_size()
{
    return interfaceStack.top()->port_name_size();
}

int RecursiveJackContext::port_type_size()
{
    return interfaceStack.top()->port_type_size();
}

const char ** RecursiveJackContext::get_ports (jack_client_t *client, const char *port_name_pattern, const char *type_name_pattern, unsigned long flags)
{
    const char ** returnValue = mapClientToInterface[client]->get_ports(client, port_name_pattern, type_name_pattern, flags);
    mapPointerToInterface[(void*)returnValue] = mapClientToInterface[client];
    return returnValue;
}

jack_port_t * RecursiveJackContext::port_by_name (jack_client_t *client, const char *port_name)
{
    jack_port_t *port = mapClientToInterface[client]->port_by_name(client, port_name);
    if (port) {
        mapPortToInterface[port] = mapClientToInterface[client];
    }
    return port;
}

jack_port_t * RecursiveJackContext::port_by_id (jack_client_t *client, jack_port_id_t port_id)
{
    jack_port_t *port = mapClientToInterface[client]->port_by_id(client, port_id);
    if (port) {
        mapPortToInterface[port] = mapClientToInterface[client];
    }
    return port;
}

jack_nframes_t RecursiveJackContext::frames_since_cycle_start (const jack_client_t *client)
{
    return mapClientToInterface[client]->frames_since_cycle_start(client);
}

jack_nframes_t RecursiveJackContext::frame_time (const jack_client_t *client)
{
    return mapClientToInterface[client]->frame_time(client);
}

jack_nframes_t RecursiveJackContext::last_frame_time (const jack_client_t *client)
{
    return mapClientToInterface[client]->last_frame_time(client);
}

jack_time_t RecursiveJackContext::frames_to_time(const jack_client_t *client, jack_nframes_t nframes)
{
    return mapClientToInterface[client]->frames_to_time(client, nframes);
}

jack_nframes_t RecursiveJackContext::time_to_frames(const jack_client_t *client, jack_time_t time)
{
    return mapClientToInterface[client]->time_to_frames(client, time);
}

jack_time_t RecursiveJackContext::get_time()
{
    return interfaceStack.top()->get_time();
}

void RecursiveJackContext::set_error_function (void (*func)(const char *))
{
    interfaceStack.top()->set_error_function(func);
}

void RecursiveJackContext::set_info_function (void (*func)(const char *))
{
    interfaceStack.top()->set_info_function(func);
}

void RecursiveJackContext::free(void* ptr)
{
    mapPointerToInterface[ptr]->free(ptr);
    mapPointerToInterface.erase(ptr);
}
