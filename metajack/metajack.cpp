#include <jack/jack.h>
#include <stdarg.h>
#include <cassert>
#include "metajackcontext.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
  All notes regard what is to be done to make meta_jack work,
  not the current state, which just passes on calls to the original JACK API.
  */

// Note: no need to reimplement this
void meta_jack_get_version(int *major_ptr, int *minor_ptr, int *micro_ptr, int *proto_ptr)
{
    jack_get_version(major_ptr, minor_ptr, micro_ptr, proto_ptr);
}

// Note: no need to reimplement this
const char * meta_jack_get_version_string()
{
    return jack_get_version_string();
}

/*
  Note: the type MetaJackClientNew (_jack_client) has to be defined.
  A new instance has to be created in the current meta_jack_context.

  Status update is not yet implemented... as is any option except JackUseExactName.
  */
MetaJackClient * meta_jack_client_open (const char *client_name, jack_options_t options, jack_status_t *, ...)
{
    //return jack_client_open(client_name, options, status);
    return MetaJackContext::getInstance()->openClient(client_name, options);
}

int meta_jack_client_close (MetaJackClient *client)
{
    return (MetaJackContext::getInstance()->closeClient(client) ? 0 : 1);
}

// Note: no real need to reimplement this
int meta_jack_client_name_size (void)
{
    return MetaJackContext::getClientNameSize();
}

/*
  Note: the client name has to be stored somewhere in the
  client's meta_jack_context. Probably the MetaJackClientNew
  should be defined such that it includes that name.
  */
char * meta_jack_get_client_name (MetaJackClient *client)
{
    return (char*)client->getName().c_str();
}

/*
  Note: after calling this, the given client has to be
  considered in meta_jack's graph execution.
  Probably this could just mean to allow the creation
  of connections to this client's ports.
  */
int meta_jack_activate (MetaJackClient *client)
{
    return (MetaJackContext::getInstance()->activateClient(client) ? 0 : 1);
}

/*
  Note: remove all connections to this client and
  forbid making any further connections to it.
  Port buffer should be left as they are.
  */
int meta_jack_deactivate (MetaJackClient *client)
{
    return (MetaJackContext::getInstance()->deactivateClient(client) ? 0 : 1);
}

/*
  Note: return the PID of the meta_jack_context of
  this client, i.e. the PID of the real JACK client
  this meta client is capsuled in.
  */
int meta_jack_get_client_pid (const char *)
{
    return MetaJackContext::getInstance()->get_pid();
}

/*
  Note: return the thread which the meta_jack_context,
  i.e. the capsuling real JACK client, is running in.
  Sidenote: multithreading inside one meta_jack_context
  is not possible this way.
  */
pthread_t meta_jack_client_thread_id (MetaJackClient *client)
{
    assert(client);
    return MetaJackContext::getInstance()->get_thread_id();
}

/*
  Note: get the given client's meta_jack_context. Then get
  the real JACK client it represents, and call jack_is_realtime
  with it.
  */
int meta_jack_is_realtime (MetaJackClient *client)
{
    assert(client);
    return MetaJackContext::getInstance()->is_realtime();
}

/*
  Note: not quite clear yet how to handle this...
  Probably just get the real client associated with the
  given client's meta_jack_context and then call the original
  implementation on it?
  */
int meta_jack_set_thread_init_callback (MetaJackClient *client, JackThreadInitCallback thread_init_callback, void *arg)
{
    MetaJackContext::getInstance()->threadInitCallbackHandler.setCallback(client, thread_init_callback, arg);
    return 0;
}

/*
  Note: the given callback has to be stored by meta_jack.
  The real client associated with this client's meta_jack_context
  should register its own callback at the real server and then (in that callback)
  call all stored callback functions.
  */
void meta_jack_on_shutdown (MetaJackClient *client, JackShutdownCallback shutdown_callback, void *arg)
{
    // if the client has an info shutdown, do not register this callback (it wouldn't be called anyway):
    if (MetaJackContext::getInstance()->shutdownCallbackHandler.find(client) == MetaJackContext::getInstance()->shutdownCallbackHandler.end()) {
        MetaJackContext::getInstance()->shutdownCallbackHandler.setCallback(client, shutdown_callback, arg);
    }
}

/*
  Note: the given callback has to be stored by meta_jack.
  The real client associated with this client's meta_jack_context
  should register its own callback at the real server and then (in that callback)
  call all stored callback functions.
  */
void meta_jack_on_info_shutdown (MetaJackClient *client, JackInfoShutdownCallback shutdown_callback, void *arg)
{
    MetaJackContext::getInstance()->infoShutdownCallbackHandler.setCallback(client, shutdown_callback, arg);
    // remove any existing "simple" shutdown callback for this client (it shouldn't be called if the client registers an info shutdown callback):
    MetaJackContext::getInstance()->shutdownCallbackHandler.erase(client);
}

/*
  Note: the given callback definitely has to be stored and later called
  by meta_jack.
  */
int meta_jack_set_process_callback (MetaJackClient *client, JackProcessCallback process_callback, void *arg)
{
    return (MetaJackContext::getInstance()->setProcessCallback(client, process_callback, arg) ? 0 : 1);
}

/*
  Note: the given callback has to be stored by meta_jack.
  The real client associated with this client's meta_jack_context
  should register its own callback at the real server and then (in that callback)
  call all stored callback functions.
  */
int meta_jack_set_freewheel_callback (MetaJackClient *client, JackFreewheelCallback freewheel_callback, void *arg)
{
    MetaJackContext::getInstance()->freewheelCallbackHandler.setCallback(client, freewheel_callback, arg);
    return 0;
}

/*
  Note: the given callback has to be stored by meta_jack.
  The real client associated with this client's meta_jack_context
  should register its own callback at the real server and then (in that callback)
  call all stored callback functions.
  */
int meta_jack_set_buffer_size_callback (MetaJackClient *client, JackBufferSizeCallback bufsize_callback, void *arg)
{
    MetaJackContext::getInstance()->bufferSizeCallbackHandler.setCallback(client, bufsize_callback, arg);
    return 0;
}

/*
  Note: the given callback has to be stored by meta_jack.
  The real client associated with this client's meta_jack_context
  should register its own callback at the real server and then (in that callback)
  call all stored callback functions.
  */
int meta_jack_set_sample_rate_callback (MetaJackClient *client, JackSampleRateCallback srate_callback, void *arg)
{
    MetaJackContext::getInstance()->sampleRateCallbackHandler.setCallback(client, srate_callback, arg);
    return 0;
}

/*
  Note: the given callback has to be stored by meta_jack.
  It should only be called when new clients register in the same
  meta_jack_context as the given client.
  */
int meta_jack_set_client_registration_callback (MetaJackClient *client, JackClientRegistrationCallback registration_callback, void *arg)
{
    MetaJackContext::getInstance()->clientRegistrationCallbackHandler.setCallback(client, registration_callback, arg);
    return 0;
}

// Note: see meta_jack_set_client_registration_callback
int meta_jack_set_port_registration_callback (MetaJackClient *client, JackPortRegistrationCallback registration_callback, void *arg)
{
    MetaJackContext::getInstance()->portRegistrationCallbackHandler.setCallback(client, registration_callback, arg);
    return 0;
}

// Note: see meta_jack_set_client_registration_callback
int meta_jack_set_port_connect_callback (MetaJackClient *client, JackPortConnectCallback connect_callback, void *arg)
{
    MetaJackContext::getInstance()->portConnectCallbackHandler.setCallback(client, connect_callback, arg);
    return 0;
}

// Note: see meta_jack_set_client_registration_callback
int meta_jack_set_port_rename_callback (MetaJackClient *client, JackPortRenameCallback rename_callback, void *arg)
{
    MetaJackContext::getInstance()->portRenameCallbackHandler.setCallback(client, rename_callback, arg);
    return 0;
}

// Note: see meta_jack_set_client_registration_callback
int meta_jack_set_graph_order_callback (MetaJackClient *client, JackGraphOrderCallback graph_callback, void *arg)
{
    MetaJackContext::getInstance()->graphOrderCallbackHandler.setCallback(client, graph_callback, arg);
    return 0;
}

/*
  Note: the given callback has to be stored by meta_jack.
  The real client associated with this client's meta_jack_context
  should register its own callback at the real server and then (in that callback)
  call all stored callback functions.
  */
int meta_jack_set_xrun_callback (MetaJackClient *client, JackXRunCallback xrun_callback, void *arg)
{
    MetaJackContext::getInstance()->xRunCallbackHandler.setCallback(client, xrun_callback, arg);
    return 0;
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
int meta_jack_set_freewheel(MetaJackClient *client, int onoff)
{
    assert(client);
    return MetaJackContext::getInstance()->set_freewheel(onoff);
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
int meta_jack_set_buffer_size (MetaJackClient *client, jack_nframes_t nframes)
{
    assert(client);
    return MetaJackContext::getInstance()->set_buffer_size(nframes);
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
jack_nframes_t meta_jack_get_sample_rate (MetaJackClient *client)
{
    assert(client);
    return MetaJackContext::getInstance()->get_sample_rate();
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
jack_nframes_t meta_jack_get_buffer_size (MetaJackClient *client)
{
    assert(client);
    return MetaJackContext::getInstance()->get_buffer_size();
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
float meta_jack_cpu_load (MetaJackClient *client)
{
    assert(client);
    return MetaJackContext::getInstance()->get_cpu_load();
}

/*
  Note: the type MetaJackPortNew (_jack_port) has to be defined.
  A new instance has to be created in the current meta_jack_context.
  A buffer has to be created for the new port.
  */
MetaJackPort * meta_jack_port_register (MetaJackClient *client, const char *port_name, const char *port_type, unsigned long flags, unsigned long buffer_size)
{
    return MetaJackContext::getInstance()->registerPort(client, port_name, port_type, flags, buffer_size);
}

/*
  Note: remove all connections from the port and free the buffer associated with the port.
  */
int meta_jack_port_unregister (MetaJackClient *client, MetaJackPort *port)
{
    assert(client);
    return (MetaJackContext::getInstance()->unregisterPort(port) ? 0 : 1);
}

/*
  Note: get the buffer which is associated to the given port in its
  client's meta_jack_context.
  */
void * meta_jack_port_get_buffer (MetaJackPort *port, jack_nframes_t nframes)
{
    return MetaJackContext::getInstance()->getPortBuffer(port, nframes);
}

/*
  Note: the port name has to be stored somewhere in the given
  port's client's meta_jack_context.
  Probably define the MetaJackPortNew type to store that name
  (or the short name, respectively, which means that the client
  name has to be appended).
  */
const char * meta_jack_port_name (const MetaJackPort *port)
{
    return port->getFullName().c_str();
}

/*
  Note: the port name has to be stored somewhere in the given
  port's client's meta_jack_context.
  Probably the MetaJackPortNew type should be defined to store that name.
  */
const char * meta_jack_port_short_name (const MetaJackPort *port)
{
    return port->getShortName().c_str();
}

/*
  Note: these flags should probably be stored in the yet-to-define
  MetaJackPortNew type.
  */
int meta_jack_port_flags (const MetaJackPort *port)
{
    return port->getFlags();
}

/*
  Note: the port type should probably be stored in the yet-to-define
  MetaJackPortNew type.
  */
const char * meta_jack_port_type (const MetaJackPort *port)
{
    return port->getType().c_str();
}

/*
  Note: a port's client should probably be stored in the yet-to-define
  MetaJackPortNew type (which makes this test trivial).
  */
int meta_jack_port_is_mine (const MetaJackClient *client, const MetaJackPort *port)
{
    return port->belongsTo(client);
}

/*
  Note: how to implement this depends on how the meta_jack_context's
  meta client graph will bedefined.
  */
int meta_jack_port_connected (const MetaJackPort *port)
{
    return port->getConnectionCount();
}

/*
  Note: how to implement this depends on how the meta_jack_context's
  meta client graph will bedefined.
  */
int meta_jack_port_connected_to (const MetaJackPort *port, const char *port_name)
{
    MetaJackPort *connectedPort = MetaJackContext::getInstance()->getPortByName(port_name);
    if (connectedPort) {
        return port->isConnectedTo(connectedPort);
    } else {
        return 0;
    }
}

/*
  Note: meta_jack_free() should be defined accordingly (such that it can
  free memory returned by this function).
  Also: how to implement this depends on how the meta_jack_context's
  meta client graph will bedefined.
  */
const char ** meta_jack_port_get_connections (const MetaJackPort *port)
{
    return port->getConnections();
}

/*
  Note: meta_jack_free() should be defined accordingly (such that it can
  free memory returned by this function).
  Also: how to implement this depends on how the meta_jack_context's
  meta client graph will bedefined.
  */
const char ** meta_jack_port_get_all_connections (const MetaJackClient *client, const MetaJackPort *port)
{
    assert(client);
    return port->getConnections();
}

/*
  Note: probably just return 0 here?
  */
jack_nframes_t meta_jack_port_get_latency (MetaJackPort *)
{
   return 0;
}

/*
  Note: not quite sure how to implement this, as no port inside
  a meta_jack_context will have the JackPortIsTerminal flag set.
  Just return 0 or pass on to the real JACK server?
  */
jack_nframes_t meta_jack_port_get_total_latency (MetaJackClient *, MetaJackPort *)
{
    return 0;
}

/*
  Note: will probably not be needed in meta_jack, so just make this
  a no-op...
  */
void meta_jack_port_set_latency (MetaJackPort *, jack_nframes_t)
{
    //jack_port_set_latency(port, nframes);
}

/*
  Note: will probably not be needed in meta_jack, so just make this
  a no-op...
  */
int meta_jack_recompute_total_latency (MetaJackClient *, MetaJackPort*)
{
    //return jack_recompute_total_latency(client, port);
    return 1;
}

/*
  Note: will probably not be needed in meta_jack, so just make this
  a no-op...
  */
int meta_jack_recompute_total_latencies (MetaJackClient *)
{
    //return jack_recompute_total_latencies(client);
    return 1;
}

/*
  Note: this depends on how MetaJackPortNew (_jack_port) is implemented.
  The callback handed to meta_jack_set_port_rename_callback should be called here if the port's
  client has registered such a callback.
  */
int meta_jack_port_set_name (MetaJackPort *port, const char *port_name)
{
    return (MetaJackContext::getInstance()->renamePort(port, port_name) ? 0 : 1);
}

/*
  Note: probably have a map of such aliases in a meta_jack_context.
  Assure that each port has two aliases at most. Also assure that
  no two ports have the same alias.
  */
int meta_jack_port_set_alias (MetaJackPort *client, const char *)
{
    assert(client);
    return 1;
}

/*
  Note: probably have a map of such aliases in a meta_jack_context.
  */
int meta_jack_port_unset_alias (MetaJackPort *client, const char *)
{
    assert(client);
    return 1;
}

/*
  Note: probably have a map of such aliases in a meta_jack_context.
  */
int meta_jack_port_get_aliases (const MetaJackPort *client, char* const [])
{
    assert(client);
    return 0;
}

/*
  Note: probably a no-op.
  */
int meta_jack_port_request_monitor (MetaJackPort *, int)
{
    return 1;
}

/*
  Note: probably a no-op.
  */
int meta_jack_port_request_monitor_by_name (MetaJackClient *, const char *, int)
{
    return 1;
}

/*
  Note: probably a no-op.
  */
int meta_jack_port_ensure_monitor (MetaJackPort *, int)
{
    return 1;
}

/*
  Note: probably a no-op.
  */
int meta_jack_port_monitoring_input (MetaJackPort *)
{
    return 0;
}

/*
  Note: how to implement this depends on how the meta_jack_context's
  meta client graph will bedefined.
  */
int meta_jack_connect (MetaJackClient *client, const char *source_port, const char *destination_port)
{
    assert(client);
    return (MetaJackContext::getInstance()->connectPorts(source_port, destination_port) ? 0 : 1);
}

/*
  Note: how to implement this depends on how the meta_jack_context's
  meta client graph will bedefined.
  */
int meta_jack_disconnect (MetaJackClient *client, const char *source_port, const char *destination_port)
{
    assert(client);
    return (MetaJackContext::getInstance()->disconnectPorts(source_port, destination_port) ? 0 : 1);
}

/*
  Note: how to implement this depends on how the meta_jack_context's
  meta client graph will bedefined.
  */
int meta_jack_port_disconnect (MetaJackClient *, MetaJackPort *)
{
    return 1;
}

// Note: no need to reimplement this
int meta_jack_port_name_size(void)
{
    return MetaJackContext::getPortNameSize();
}

// Note: no need to reimplement this
int meta_jack_port_type_size(void)
{
    return MetaJackContext::getPortTypeSize();
}

/*
  Note: what regular expression implementation could be used here?
  meta_jack_free() should be defined accordingly (such that it can
  free memory returned by this function).
  Also: how to implement this depends on how the meta_jack_context's
  meta client graph will bedefined.
  */
const char ** meta_jack_get_ports (MetaJackClient *client, const char *port_name_pattern, const char *type_name_pattern, unsigned long flags)
{
    //return jack_get_ports(client, port_name_pattern, type_name_pattern, flags);
    if (client) {
        std::string port_name_pattern_string = (port_name_pattern ? port_name_pattern : "");
        std::string type_name_pattern_string = (type_name_pattern ? type_name_pattern : "");
        return MetaJackContext::getInstance()->getPortsByPattern(port_name_pattern_string, type_name_pattern_string, flags);
    } else {
        return 0;
    }
}

/*
  Note: look through the given client's ports to see which one has the given name.
  */
MetaJackPort * meta_jack_port_by_name (MetaJackClient *client, const char *port_name)
{
    assert(client);
    return MetaJackContext::getInstance()->getPortByName(port_name);
}

/*
  Note: look through the given client's ports to see which one has the given id.
  */
MetaJackPort * meta_jack_port_by_id (MetaJackClient *client, jack_port_id_t port_id)
{
    assert(client);
    return MetaJackContext::getInstance()->getPortById(port_id);
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
jack_nframes_t meta_jack_frames_since_cycle_start (const MetaJackClient *client)
{
    assert(client);
    return MetaJackContext::getInstance()->get_frames_since_cycle_start();
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
jack_nframes_t meta_jack_frame_time (const MetaJackClient *client)
{
    assert(client);
    return MetaJackContext::getInstance()->get_frame_time();
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
jack_nframes_t meta_jack_last_frame_time (const MetaJackClient *client)
{
    assert(client);
    return MetaJackContext::getInstance()->get_last_frame_time();
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
jack_time_t meta_jack_frames_to_time(const MetaJackClient *client, jack_nframes_t nframes)
{
    assert(client);
    return MetaJackContext::getInstance()->convert_frames_to_time(nframes);
}

/*
  Note: get the real client associated with the given client's
  meta_jack_context and pass on the call using that client.
  */
jack_nframes_t meta_jack_time_to_frames(const MetaJackClient *client, jack_time_t time)
{
    assert(client);
    return MetaJackContext::getInstance()->convert_time_to_frames(time);
}

// Note: probably no need to reimplement this
jack_time_t meta_jack_get_time()
{
    return MetaJackContext::getInstance()->get_time();
}

/*
  Note: should clients capsuled in a meta_jack_context receive error messages?
  */
void meta_jack_set_error_function (void (*)(const char *))
{
}

/*
  Note: should clients capsuled in a meta_jack_context receive error messages?
  */
void meta_jack_set_info_function (void (*)(const char *))
{
}

/*
  Note: this has to be able to free memory pointed to by pointers
  returned by the following functions:
  - meta_jack_port_get_connections()
  - meta_jack_port_get_all_connections()
  - meta_jack_get_ports
  */
void meta_jack_free(void* ptr)
{
    MetaJackContext::getInstance()->free(ptr);
}

#ifdef __cplusplus
}
#endif
