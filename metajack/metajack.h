#ifndef META_JACK_H
#define META_JACK_H

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

/**
  This is just a stub which can be used instead of the original <jack/jack.h>
  header to not use the real JACK server but capsule all clients in one
  meta_jack_context (they will appear as a single client to the JACK server).
  */

#ifdef __cplusplus
extern "C"
{
#endif

#include <jack/types.h>
#include <jack/systemdeps.h>
#include <jack/weakmacros.h>

jack_client_t * meta_jack_client_by_name(const char *client_name);

// from Jack API "jack.h":
#define jack_get_version                        meta_jack_get_version
#define jack_get_version_string                 meta_jack_get_version_string
#define jack_client_open                        meta_jack_client_open
#define jack_client_close                       meta_jack_client_close
#define jack_client_name_size                   meta_jack_client_name_size
#define jack_get_client_name                    meta_jack_get_client_name
// Note: internal clients cannot be capsuled and these are deprecated anyway:
//#define jack_internal_client_new                meta_jack_internal_client_new
//#define jack_internal_client_close              meta_jack_internal_client_close
#define jack_activate                           meta_jack_activate
#define jack_deactivate                         meta_jack_deactivate
#define jack_get_client_pid                     meta_jack_get_client_pid
#define jack_client_thread_id                   meta_jack_client_thread_id
#define jack_is_realtime                        meta_jack_is_realtime
// Note: non-callback API will not be supported by meta_jack:
//#define jack_cycle_wait                         meta_jack_cycle_wait
//#define jack_cycle_signal                       meta_jack_cycle_signal
//#define jack_set_process_thread                 meta_jack_set_process_thread
#define jack_set_thread_init_callback           meta_jack_set_thread_init_callback
#define jack_on_shutdown                        meta_jack_on_shutdown
#define jack_on_info_shutdown                   meta_jack_on_info_shutdown
#define jack_set_process_callback               meta_jack_set_process_callback
#define jack_set_freewheel_callback             meta_jack_set_freewheel_callback
#define jack_set_buffer_size_callback           meta_jack_set_buffer_size_callback
#define jack_set_sample_rate_callback           meta_jack_set_sample_rate_callback
#define jack_set_client_registration_callback   meta_jack_set_client_registration_callback
#define jack_set_port_registration_callback     meta_jack_set_port_registration_callback
#define jack_set_port_connect_callback          meta_jack_set_port_connect_callback
#define jack_set_port_rename_callback           meta_jack_set_port_rename_callback
#define jack_set_graph_order_callback           meta_jack_set_graph_order_callback
#define jack_set_xrun_callback                  meta_jack_set_xrun_callback
#define jack_set_freewheel                      meta_jack_set_freewheel
#define jack_set_buffer_size                    meta_jack_set_buffer_size
#define jack_get_sample_rate                    meta_jack_get_sample_rate
#define jack_get_buffer_size                    meta_jack_get_buffer_size
#define jack_cpu_load                           meta_jack_cpu_load
#define jack_port_register                      meta_jack_port_register
#define jack_port_unregister                    meta_jack_port_unregister
#define jack_port_get_buffer                    meta_jack_port_get_buffer
#define jack_port_name                          meta_jack_port_name
#define jack_port_short_name                    meta_jack_port_short_name
#define jack_port_flags                         meta_jack_port_flags
#define jack_port_type                          meta_jack_port_type
#define jack_port_type_id                       meta_jack_port_type_id
#define jack_port_is_mine                       meta_jack_port_is_mine
#define jack_port_connected                     meta_jack_port_connected
#define jack_port_connected_to                  meta_jack_port_connected_to
#define jack_port_get_connections               meta_jack_port_get_connections
#define jack_port_get_all_connections           meta_jack_port_get_all_connections
#define jack_port_get_latency                   meta_jack_port_get_latency
#define jack_port_get_total_latency             meta_jack_port_get_total_latency
#define jack_port_set_latency                   meta_jack_port_set_latency
#define jack_recompute_total_latency            meta_jack_recompute_total_latency
#define jack_recompute_total_latencies          meta_jack_recompute_total_latencies
#define jack_port_set_name                      meta_jack_port_set_name
#define jack_port_set_alias                     meta_jack_port_set_alias
#define jack_port_unset_alias                   meta_jack_port_unset_alias
#define jack_port_get_aliases                   meta_jack_port_get_aliases
#define jack_port_request_monitor               meta_jack_port_request_monitor
#define jack_port_request_monitor_by_name       meta_jack_port_request_monitor_by_name
#define jack_port_ensure_monitor                meta_jack_port_ensure_monitor
#define jack_port_monitoring_input              meta_jack_port_monitoring_input
#define jack_connect                            meta_jack_connect
#define jack_disconnect                         meta_jack_disconnect
#define jack_port_disconnect                    meta_jack_port_disconnect
#define jack_port_name_size                     meta_jack_port_name_size
#define jack_port_type_size                     meta_jack_port_type_size
#define jack_get_ports                          meta_jack_get_ports
#define jack_port_by_name                       meta_jack_port_by_name
#define jack_port_by_id                         meta_jack_port_by_id
#define jack_frames_since_cycle_start           meta_jack_frames_since_cycle_start
#define jack_frame_time                         meta_jack_frame_time
#define jack_last_frame_time                    meta_jack_last_frame_time
#define jack_frames_to_time                     meta_jack_frames_to_time
#define jack_time_to_frames                     meta_jack_time_to_frames
#define jack_get_time                           meta_jack_get_time
#define jack_set_error_function                 meta_jack_set_error_function
#define jack_set_info_function                  meta_jack_set_info_function
#define jack_free                               meta_jack_free
// from Jack API "transport.h":
#define jack_release_timebase                   meta_jack_release_timebase
#define jack_set_sync_callback                  meta_jack_set_sync_callback
#define jack_set_sync_timeout                   meta_jack_set_sync_timeout
#define jack_set_timebase_callback              meta_jack_set_timebase_callback
#define jack_transport_locate                   meta_jack_transport_locate
#define jack_transport_query                    meta_jack_transport_query
#define jack_get_current_transport_frame        meta_jack_get_current_transport_frame
#define jack_transport_reposition               meta_jack_transport_reposition
#define jack_transport_start                    meta_jack_transport_start
#define jack_transport_stop                     meta_jack_transport_stop
#define jack_get_transport_info                 meta_jack_get_transport_info
#define jack_set_transport_info                 meta_jack_set_transport_info


/**
 * Call this function to get version of the JACK, in form of several numbers
 *
 * @param major_ptr pointer to variable receiving major version of JACK.
 *
 * @param minor_ptr pointer to variable receiving minor version of JACK.
 *
 * @param major_ptr pointer to variable receiving micro version of JACK.
 *
 * @param major_ptr pointer to variable receiving protocol version of JACK.
 *
 */
void
meta_jack_get_version(
    int *major_ptr,
    int *minor_ptr,
    int *micro_ptr,
    int *proto_ptr) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Call this function to get version of the JACK, in form of a string
 *
 * @return Human readable string describing JACK version being used.
 *
 */
const char *
meta_jack_get_version_string() JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Open an external client session with a JACK server.  This interface
 * is more complex but more powerful than jack_client_new().  With it,
 * clients may choose which of several servers to connect, and control
 * whether and how to start the server automatically, if it was not
 * already running.  There is also an option for JACK to generate a
 * unique client name, when necessary.
 *
 * @param client_name of at most jack_client_name_size() characters.
 * The name scope is local to each server.  Unless forbidden by the
 * @ref JackUseExactName option, the server will modify this name to
 * create a unique variant, if needed.
 *
 * @param options formed by OR-ing together @ref JackOptions bits.
 * Only the @ref JackOpenOptions bits are allowed.
 *
 * @param status (if non-NULL) an address for JACK to return
 * information from the open operation.  This status word is formed by
 * OR-ing together the relevant @ref JackStatus bits.
 *
 *
 * <b>Optional parameters:</b> depending on corresponding [@a options
 * bits] additional parameters may follow @a status (in this order).
 *
 * @arg [@ref JackServerName] <em>(char *) server_name</em> selects
 * from among several possible concurrent server instances.  Server
 * names are unique to each user.  If unspecified, use "default"
 * unless \$JACK_DEFAULT_SERVER is defined in the process environment.
 *
 * @return Opaque client handle if successful.  If this is NULL, the
 * open operation failed, @a *status includes @ref JackFailure and the
 * caller is not a JACK client.
 */
jack_client_t * meta_jack_client_open (const char *client_name,
                                  jack_options_t options,
                                  jack_status_t *status, ...) JACK_OPTIONAL_WEAK_EXPORT;

///**
//* \bold THIS FUNCTION IS DEPRECATED AND SHOULD NOT BE USED IN
//*  NEW JACK CLIENTS
//*
//* @deprecated Please use jack_client_open().
//*/
//jack_client_t * meta_jack_client_new (const char *client_name) JACK_OPTIONAL_WEAK_DEPRECATED_EXPORT;

/**
 * Disconnects an external client from a JACK server.
 *
 * @return 0 on success, otherwise a non-zero error code
 */
int meta_jack_client_close (jack_client_t *client) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @return the maximum number of characters in a JACK client name
 * including the final NULL character.  This value is a constant.
 */
int meta_jack_client_name_size (void) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @return pointer to actual client name.  This is useful when @ref
 * JackUseExactName is not specified on open and @ref
 * JackNameNotUnique status was returned.  In that case, the actual
 * name will differ from the @a client_name requested.
 */
char * meta_jack_get_client_name (jack_client_t *client) JACK_OPTIONAL_WEAK_EXPORT;

///**
// * Load an internal client into the Jack server.
// *
// * Internal clients run inside the JACK server process.  They can use
// * most of the same functions as external clients.  Each internal
// * client must declare jack_initialize() and jack_finish() entry
// * points, called at load and unload times.  See inprocess.c for an
// * example of how to write an internal client.
// *
// * @deprecated Please use jack_internal_client_load().
// *
// * @param client_name of at most jack_client_name_size() characters.
// *
// * @param load_name of a shared object file containing the code for
// * the new client.
// *
// * @param load_init an arbitary string passed to the jack_initialize()
// * routine of the new client (may be NULL).
// *
// * @return 0 if successful.
// */
//int meta_jack_internal_client_new (const char *client_name,
//                              const char *load_name,
//                              const char *load_init) JACK_OPTIONAL_WEAK_DEPRECATED_EXPORT;

///**
// * Remove an internal client from a JACK server.
// *
// * @deprecated Please use jack_internal_client_load().
// */
//void meta_jack_internal_client_close (const char *client_name) JACK_OPTIONAL_WEAK_DEPRECATED_EXPORT;

/**
 * Tell the Jack server that the program is ready to start processing
 * audio.
 *
 * @return 0 on success, otherwise a non-zero error code
 */
int meta_jack_activate (jack_client_t *client) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Tell the Jack server to remove this @a client from the process
 * graph.  Also, disconnect all ports belonging to it, since inactive
 * clients have no port connections.
 *
 * @return 0 on success, otherwise a non-zero error code
 */
int meta_jack_deactivate (jack_client_t *client) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @return pid of client. If not available, 0 will be returned.
 */
int meta_jack_get_client_pid (const char *name) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @return the pthread ID of the thread running the JACK client side
 * code.
 */
pthread_t meta_jack_client_thread_id (jack_client_t *) JACK_OPTIONAL_WEAK_EXPORT;

/*@}*/

/**
 * @param client pointer to JACK client structure.
 *
 * Check if the JACK subsystem is running with -R (--realtime).
 *
 * @return 1 if JACK is running realtime, 0 otherwise
 */
int meta_jack_is_realtime (jack_client_t *client) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @defgroup NonCallbackAPI The non-callback API
 * @{
 */

///**
// * \bold THIS FUNCTION IS DEPRECATED AND SHOULD NOT BE USED IN
// *  NEW JACK CLIENTS.
// *
// * @deprecated Please use jack_cycle_wait() and jack_cycle_signal() functions.
// */
//jack_nframes_t jack_thread_wait (jack_client_t*, int status) JACK_OPTIONAL_WEAK_EXPORT;

///**
// * Wait until this JACK client should process data.
// *
// * @param client - pointer to a JACK client structure
// *
// * @return the number of frames of data to process
// */
//jack_nframes_t meta_jack_cycle_wait (jack_client_t* client) JACK_OPTIONAL_WEAK_EXPORT;

///**
// * Signal next clients in the graph.
// *
// * @param client - pointer to a JACK client structure
// * @param status - if non-zero, calling thread should exit
// */
//void meta_jack_cycle_signal (jack_client_t* client, int status) JACK_OPTIONAL_WEAK_EXPORT;

///**
// * Tell the Jack server to call @a thread_callback in the RT thread.
// * Typical use are in conjunction with @a jack_cycle_wait and @a jack_cycle_signal functions.
// * The code in the supplied function must be suitable for real-time
// * execution.  That means that it cannot call functions that might
// * block for a long time. This includes malloc, free, printf,
// * pthread_mutex_lock, sleep, wait, poll, select, pthread_join,
// * pthread_cond_wait, etc, etc. See
// * http://jackit.sourceforge.net/docs/design/design.html#SECTION00411000000000000000
// * for more information.
// *
// * NOTE: this function cannot be called while the client is activated
// * (after jack_activate has been called.)
// *
// * @return 0 on success, otherwise a non-zero error code.
//*/
//int meta_jack_set_process_thread(jack_client_t* client, JackThreadCallback thread_callback, void *arg) JACK_OPTIONAL_WEAK_EXPORT;

/*@}*/

/**
 * @defgroup ClientCallbacks Setting Client Callbacks
 * @{
 */

/**
 * Tell JACK to call @a thread_init_callback once just after
 * the creation of the thread in which all other callbacks
 * will be handled.
 *
 * The code in the supplied function does not need to be
 * suitable for real-time execution.
 *
 * NOTE: this function cannot be called while the client is activated
 * (after jack_activate has been called.)
 *
 * @return 0 on success, otherwise a non-zero error code, causing JACK
 * to remove that client from the process() graph.
 */
int meta_jack_set_thread_init_callback (jack_client_t *client,
                                   JackThreadInitCallback thread_init_callback,
                                   void *arg) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @param client pointer to JACK client structure.
 * @param function The jack_shutdown function pointer.
 * @param arg The arguments for the jack_shutdown function.
 *
 * Register a function (and argument) to be called if and when the
 * JACK server shuts down the client thread.  The function must
 * be written as if it were an asynchonrous POSIX signal
 * handler --- use only async-safe functions, and remember that it
 * is executed from another thread.  A typical function might
 * set a flag or write to a pipe so that the rest of the
 * application knows that the JACK client thread has shut
 * down.
 *
 * NOTE: clients do not need to call this.  It exists only
 * to help more complex clients understand what is going
 * on.  It should be called before jack_client_activate().
 *
 * NOTE: if a client calls this AND jack_on_info_shutdown(), then
 * the event of a client thread shutdown, the callback
 * passed to this function will not be called, and the one passed to
 * jack_on_info_shutdown() will.
 */
void meta_jack_on_shutdown (jack_client_t *client,
                       JackShutdownCallback shutdown_callback, void *arg) JACK_WEAK_EXPORT;

/**
 * @param client pointer to JACK client structure.
 * @param function The jack_info_shutdown function pointer.
 * @param arg The arguments for the jack_info_shutdown function.
 *
 * Register a function (and argument) to be called if and when the
 * JACK server shuts down the client thread.  The function must
 * be written as if it were an asynchonrous POSIX signal
 * handler --- use only async-safe functions, and remember that it
 * is executed from another thread.  A typical function might
 * set a flag or write to a pipe so that the rest of the
 * application knows that the JACK client thread has shut
 * down.
 *
 * NOTE: clients do not need to call this.  It exists only
 * to help more complex clients understand what is going
 * on.  It should be called before jack_client_activate().
 *
 * NOTE: if a client calls this AND jack_on_info_shutdown(), then
 * the event of a client thread shutdown, the callback
 * passed to this function will not be called, and the one passed to
 * jack_on_info_shutdown() will.
 */
void meta_jack_on_info_shutdown (jack_client_t *client,
                            JackInfoShutdownCallback shutdown_callback, void *arg) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Tell the Jack server to call @a process_callback whenever there is
 * work be done, passing @a arg as the second argument.
 *
 * The code in the supplied function must be suitable for real-time
 * execution.  That means that it cannot call functions that might
 * block for a long time. This includes malloc, free, printf,
 * pthread_mutex_lock, sleep, wait, poll, select, pthread_join,
 * pthread_cond_wait, etc, etc. See
 * http://jackit.sourceforge.net/docs/design/design.html#SECTION00411000000000000000
 * for more information.
 *
 * NOTE: this function cannot be called while the client is activated
 * (after jack_activate has been called.)
 *
 * @return 0 on success, otherwise a non-zero error code.
 */
int meta_jack_set_process_callback (jack_client_t *client,
                               JackProcessCallback process_callback,
                               void *arg) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Tell the Jack server to call @a freewheel_callback
 * whenever we enter or leave "freewheel" mode, passing @a
 * arg as the second argument. The first argument to the
 * callback will be non-zero if JACK is entering freewheel
 * mode, and zero otherwise.
 *
 * All "notification events" are received in a seperated non RT thread,
 * the code in the supplied function does not need to be
 * suitable for real-time execution.
 *
 * NOTE: this function cannot be called while the client is activated
 * (after jack_activate has been called.)
 *
 * @return 0 on success, otherwise a non-zero error code.
 */
int meta_jack_set_freewheel_callback (jack_client_t *client,
                                 JackFreewheelCallback freewheel_callback,
                                 void *arg) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Tell JACK to call @a bufsize_callback whenever the size of the the
 * buffer that will be passed to the @a process_callback is about to
 * change.  Clients that depend on knowing the buffer size must supply
 * a @a bufsize_callback before activating themselves.
 *
 * All "notification events" are received in a seperated non RT thread,
 * the code in the supplied function does not need to be
 * suitable for real-time execution.
 *
 * NOTE: this function cannot be called while the client is activated
 * (after jack_activate has been called.)
 *
 * @param client pointer to JACK client structure.
 * @param bufsize_callback function to call when the buffer size changes.
 * @param arg argument for @a bufsize_callback.
 *
 * @return 0 on success, otherwise a non-zero error code
 */
int meta_jack_set_buffer_size_callback (jack_client_t *client,
                                   JackBufferSizeCallback bufsize_callback,
                                   void *arg) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Tell the Jack server to call @a srate_callback whenever the system
 * sample rate changes.
 *
 * All "notification events" are received in a seperated non RT thread,
 * the code in the supplied function does not need to be
 * suitable for real-time execution.
 *
 * NOTE: this function cannot be called while the client is activated
 * (after jack_activate has been called.)
 *
 * @return 0 on success, otherwise a non-zero error code
 */
int meta_jack_set_sample_rate_callback (jack_client_t *client,
                                   JackSampleRateCallback srate_callback,
                                   void *arg) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Tell the JACK server to call @a client_registration_callback whenever a
 * client is registered or unregistered, passing @a arg as a parameter.
 *
 * All "notification events" are received in a seperated non RT thread,
 * the code in the supplied function does not need to be
 * suitable for real-time execution.
 *
 * NOTE: this function cannot be called while the client is activated
 * (after jack_activate has been called.)
 *
 * @return 0 on success, otherwise a non-zero error code
 */
int meta_jack_set_client_registration_callback (jack_client_t *,
                       JackClientRegistrationCallback
                                           registration_callback, void *arg) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Tell the JACK server to call @a registration_callback whenever a
 * port is registered or unregistered, passing @a arg as a parameter.
 *
 * All "notification events" are received in a seperated non RT thread,
 * the code in the supplied function does not need to be
 * suitable for real-time execution.
 *
 * NOTE: this function cannot be called while the client is activated
 * (after jack_activate has been called.)
 *
 * @return 0 on success, otherwise a non-zero error code
 */
int meta_jack_set_port_registration_callback (jack_client_t *,
                                          JackPortRegistrationCallback
                                          registration_callback, void *arg) JACK_OPTIONAL_WEAK_EXPORT;

 /**
 * Tell the JACK server to call @a connect_callback whenever a
 * port is connected or disconnected, passing @a arg as a parameter.
 *
 * All "notification events" are received in a seperated non RT thread,
 * the code in the supplied function does not need to be
 * suitable for real-time execution.
 *
 * NOTE: this function cannot be called while the client is activated
 * (after jack_activate has been called.)
 *
 * @return 0 on success, otherwise a non-zero error code
 */
int meta_jack_set_port_connect_callback (jack_client_t *,
                                    JackPortConnectCallback
                                    connect_callback, void *arg) JACK_OPTIONAL_WEAK_EXPORT;

 /**
 * Tell the JACK server to call @a rename_callback whenever a
 * port is renamed, passing @a arg as a parameter.
 *
 * All "notification events" are received in a seperated non RT thread,
 * the code in the supplied function does not need to be
 * suitable for real-time execution.
 *
 * NOTE: this function cannot be called while the client is activated
 * (after jack_activate has been called.)
 *
 * @return 0 on success, otherwise a non-zero error code
 */
int meta_jack_set_port_rename_callback (jack_client_t *,
                                   JackPortRenameCallback
                                   rename_callback, void *arg) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Tell the JACK server to call @a graph_callback whenever the
 * processing graph is reordered, passing @a arg as a parameter.
 *
 * All "notification events" are received in a seperated non RT thread,
 * the code in the supplied function does not need to be
 * suitable for real-time execution.
 *
 * NOTE: this function cannot be called while the client is activated
 * (after jack_activate has been called.)
 *
 * @return 0 on success, otherwise a non-zero error code
 */
int meta_jack_set_graph_order_callback (jack_client_t *,
                                   JackGraphOrderCallback graph_callback,
                                   void *) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Tell the JACK server to call @a xrun_callback whenever there is a
 * xrun, passing @a arg as a parameter.
 *
 * All "notification events" are received in a seperated non RT thread,
 * the code in the supplied function does not need to be
 * suitable for real-time execution.
 *
 * NOTE: this function cannot be called while the client is activated
 * (after jack_activate has been called.)
 *
 * @return 0 on success, otherwise a non-zero error code
 */
int meta_jack_set_xrun_callback (jack_client_t *,
                            JackXRunCallback xrun_callback, void *arg) JACK_OPTIONAL_WEAK_EXPORT;

/*@}*/

/**
 * @defgroup ServerClientControl Controlling & querying JACK server operation
 * @{
 */

/**
 * Start/Stop JACK's "freewheel" mode.
 *
 * When in "freewheel" mode, JACK no longer waits for
 * any external event to begin the start of the next process
 * cycle.
 *
 * As a result, freewheel mode causes "faster than realtime"
 * execution of a JACK graph. If possessed, real-time
 * scheduling is dropped when entering freewheel mode, and
 * if appropriate it is reacquired when stopping.
 *
 * IMPORTANT: on systems using capabilities to provide real-time
 * scheduling (i.e. Linux kernel 2.4), if onoff is zero, this function
 * must be called from the thread that originally called jack_activate().
 * This restriction does not apply to other systems (e.g. Linux kernel 2.6
 * or OS X).
 *
 * @param client pointer to JACK client structure
 * @param onoff  if non-zero, freewheel mode starts. Otherwise
 *                  freewheel mode ends.
 *
 * @return 0 on success, otherwise a non-zero error code.
 */
int meta_jack_set_freewheel(jack_client_t* client, int onoff) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Change the buffer size passed to the @a process_callback.
 *
 * This operation stops the JACK engine process cycle, then calls all
 * registered @a bufsize_callback functions before restarting the
 * process cycle.  This will cause a gap in the audio flow, so it
 * should only be done at appropriate stopping points.
 *
 * @see jack_set_buffer_size_callback()
 *
 * @param client pointer to JACK client structure.
 * @param nframes new buffer size.  Must be a power of two.
 *
 * @return 0 on success, otherwise a non-zero error code
 */
int meta_jack_set_buffer_size (jack_client_t *client, jack_nframes_t nframes) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @return the sample rate of the jack system, as set by the user when
 * jackd was started.
 */
jack_nframes_t meta_jack_get_sample_rate (jack_client_t *) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @return the current maximum size that will ever be passed to the @a
 * process_callback.  It should only be used *before* the client has
 * been activated.  This size may change, clients that depend on it
 * must register a @a bufsize_callback so they will be notified if it
 * does.
 *
 * @see jack_set_buffer_size_callback()
 */
jack_nframes_t meta_jack_get_buffer_size (jack_client_t *) JACK_OPTIONAL_WEAK_EXPORT;

///**
// * Old-style interface to become the timebase for the entire JACK
// * subsystem.
// *
// * @deprecated This function still exists for compatibility with the
// * earlier transport interface, but it does nothing.  Instead, see
// * transport.h and use jack_set_timebase_callback().
// *
// * @return ENOSYS, function not implemented.
// */
//int jack_engine_takeover_timebase (jack_client_t *) JACK_OPTIONAL_WEAK_DEPRECATED_EXPORT;

/**
 * @return the current CPU load estimated by JACK.  This is a running
 * average of the time it takes to execute a full process cycle for
 * all clients as a percentage of the real time available per cycle
 * determined by the buffer size and sample rate.
 */
float meta_jack_cpu_load (jack_client_t *client) JACK_OPTIONAL_WEAK_EXPORT;

/*@}*/

/**
 * @defgroup PortFunctions Creating & manipulating ports
 * @{
 */

/**
 * Create a new port for the client. This is an object used for moving
 * data of any type in or out of the client.  Ports may be connected
 * in various ways.
 *
 * Each port has a short name.  The port's full name contains the name
 * of the client concatenated with a colon (:) followed by its short
 * name.  The jack_port_name_size() is the maximum length of this full
 * name.  Exceeding that will cause the port registration to fail and
 * return NULL.
 *
 * All ports have a type, which may be any non-NULL and non-zero
 * length string, passed as an argument.  Some port types are built
 * into the JACK API, currently only JACK_DEFAULT_AUDIO_TYPE.
 *
 * @param client pointer to JACK client structure.
 * @param port_name non-empty short name for the new port (not
 * including the leading @a "client_name:").
 * @param port_type port type name.  If longer than
 * jack_port_type_size(), only that many characters are significant.
 * @param flags @ref JackPortFlags bit mask.
 * @param buffer_size must be non-zero if this is not a built-in @a
 * port_type.  Otherwise, it is ignored.
 *
 * @return jack_port_t pointer on success, otherwise NULL.
 */
jack_port_t * meta_jack_port_register (jack_client_t *client,
                                  const char *port_name,
                                  const char *port_type,
                                  unsigned long flags,
                                  unsigned long buffer_size) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Remove the port from the client, disconnecting any existing
 * connections.
 *
 * @return 0 on success, otherwise a non-zero error code
 */
int meta_jack_port_unregister (jack_client_t *, jack_port_t *) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * This returns a pointer to the memory area associated with the
 * specified port. For an output port, it will be a memory area
 * that can be written to; for an input port, it will be an area
 * containing the data from the port's connection(s), or
 * zero-filled. if there are multiple inbound connections, the data
 * will be mixed appropriately.
 *
 * FOR OUTPUT PORTS ONLY : DEPRECATED in Jack 2.0 !!
 * ---------------------------------------------------
 * You may cache the value returned, but only between calls to
 * your "blocksize" callback. For this reason alone, you should
 * either never cache the return value or ensure you have
 * a "blocksize" callback and be sure to invalidate the cached
 * address from there.
 *
 * Caching output ports is DEPRECATED in Jack 2.0, due to some new optimization (like "pipelining").
 * Port buffers have to be retrieved in each callback for proper functionning.
 */
void * meta_jack_port_get_buffer (jack_port_t *, jack_nframes_t) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @return the full name of the jack_port_t (including the @a
 * "client_name:" prefix).
 *
 * @see jack_port_name_size().
 */
const char * meta_jack_port_name (const jack_port_t *port) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @return the short name of the jack_port_t (not including the @a
 * "client_name:" prefix).
 *
 * @see jack_port_name_size().
 */
const char * meta_jack_port_short_name (const jack_port_t *port) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @return the @ref JackPortFlags of the jack_port_t.
 */
int meta_jack_port_flags (const jack_port_t *port) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @return the @a port type, at most jack_port_type_size() characters
 * including a final NULL.
 */
const char * meta_jack_port_type (const jack_port_t *port) JACK_OPTIONAL_WEAK_EXPORT;

 /**
 * @return the @a port type id.
 */
jack_port_type_id_t meta_jack_port_type_id (const jack_port_t *port) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @return TRUE if the jack_port_t belongs to the jack_client_t.
 */
int meta_jack_port_is_mine (const jack_client_t *, const jack_port_t *port) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @return number of connections to or from @a port.
 *
 * @pre The calling client must own @a port.
 */
int meta_jack_port_connected (const jack_port_t *port) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @return TRUE if the locally-owned @a port is @b directly connected
 * to the @a port_name.
 *
 * @see jack_port_name_size()
 */
int meta_jack_port_connected_to (const jack_port_t *port,
                            const char *port_name) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @return a null-terminated array of full port names to which the @a
 * port is connected.  If none, returns NULL.
 *
 * The caller is responsible for calling jack_free(3) on any non-NULL
 * returned value.
 *
 * @param port locally owned jack_port_t pointer.
 *
 * @see jack_port_name_size(), jack_port_get_all_connections()
 */
const char ** meta_jack_port_get_connections (const jack_port_t *port) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @return a null-terminated array of full port names to which the @a
 * port is connected.  If none, returns NULL.
 *
 * The caller is responsible for calling jack_free(3) on any non-NULL
 * returned value.
 *
 * This differs from jack_port_get_connections() in two important
 * respects:
 *
 *     1) You may not call this function from code that is
 *          executed in response to a JACK event. For example,
 *          you cannot use it in a GraphReordered handler.
 *
 *     2) You need not be the owner of the port to get information
 *          about its connections.
 *
 * @see jack_port_name_size()
 */
const char ** meta_jack_port_get_all_connections (const jack_client_t *client,
                                             const jack_port_t *port) JACK_OPTIONAL_WEAK_EXPORT;

///**
// *
// * @deprecated This function will be removed from a future version
// * of JACK. Do not use it. There is no replacement. It has
// * turned out to serve essentially no purpose in real-life
// * JACK clients.
// */
//int jack_port_tie (jack_port_t *src, jack_port_t *dst) JACK_OPTIONAL_WEAK_DEPRECATED_EXPORT;

///**
// *
// * @deprecated This function will be removed from a future version
// * of JACK. Do not use it. There is no replacement. It has
// * turned out to serve essentially no purpose in real-life
// * JACK clients.
// */
//int jack_port_untie (jack_port_t *port) JACK_OPTIONAL_WEAK_DEPRECATED_EXPORT;

 /**
 * @return the time (in frames) between data being available or
 * delivered at/to a port, and the time at which it arrived at or is
 * delivered to the "other side" of the port.  E.g. for a physical
 * audio output port, this is the time between writing to the port and
 * when the signal will leave the connector.  For a physical audio
 * input port, this is the time between the sound arriving at the
 * connector and the corresponding frames being readable from the
 * port.
 */
jack_nframes_t meta_jack_port_get_latency (jack_port_t *port) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * The maximum of the sum of the latencies in every
 * connection path that can be drawn between the port and other
 * ports with the @ref JackPortIsTerminal flag set.
 */
jack_nframes_t meta_jack_port_get_total_latency (jack_client_t *,
                                            jack_port_t *port) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * The port latency is zero by default. Clients that control
 * physical hardware with non-zero latency should call this
 * to set the latency to its correct value. Note that the value
 * should include any systemic latency present "outside" the
 * physical hardware controlled by the client. For example,
 * for a client controlling a digital audio interface connected
 * to an external digital converter, the latency setting should
 * include both buffering by the audio interface *and* the converter.
 */
void meta_jack_port_set_latency (jack_port_t *, jack_nframes_t) JACK_OPTIONAL_WEAK_EXPORT;

/**
* Request a complete recomputation of a port's total latency. This
* can be called by a client that has just changed the internal
* latency of its port using @function jack_port_set_latency
* and wants to ensure that all signal pathways in the graph
* are updated with respect to the values that will be returned
* by @function jack_port_get_total_latency.
*
* @return zero for successful execution of the request. non-zero
*         otherwise.
*/
int meta_jack_recompute_total_latency (jack_client_t*, jack_port_t* port) JACK_OPTIONAL_WEAK_EXPORT;

/**
* Request a complete recomputation of all port latencies. This
* can be called by a client that has just changed the internal
* latency of its port using @function jack_port_set_latency
* and wants to ensure that all signal pathways in the graph
* are updated with respect to the values that will be returned
* by @function jack_port_get_total_latency. It allows a client
* to change multiple port latencies without triggering a
* recompute for each change.
*
* @return zero for successful execution of the request. non-zero
*         otherwise.
*/
int meta_jack_recompute_total_latencies (jack_client_t*) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Modify a port's short name.  May be called at any time.  If the
 * resulting full name (including the @a "client_name:" prefix) is
 * longer than jack_port_name_size(), it will be truncated.
 *
 * @return 0 on success, otherwise a non-zero error code.
 */
int meta_jack_port_set_name (jack_port_t *port, const char *port_name) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Set @a alias as an alias for @a port.  May be called at any time.
 * If the alias is longer than jack_port_name_size(), it will be truncated.
 *
 * After a successful call, and until JACK exits or
 * @function jack_port_unset_alias() is called, @alias may be
 * used as a alternate name for the port.
 *
 * Ports can have up to two aliases - if both are already
 * set, this function will return an error.
 *
 * @return 0 on success, otherwise a non-zero error code.
 */
int meta_jack_port_set_alias (jack_port_t *port, const char *alias) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Remove @a alias as an alias for @a port.  May be called at any time.
 *
 * After a successful call, @a alias can no longer be
 * used as a alternate name for the port.
 *
 * @return 0 on success, otherwise a non-zero error code.
 */
int meta_jack_port_unset_alias (jack_port_t *port, const char *alias) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Get any aliases known for @port.
 *
 * @return the number of aliases discovered for the port
 */
int meta_jack_port_get_aliases (const jack_port_t *port, char* const aliases[2]) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * If @ref JackPortCanMonitor is set for this @a port, turn input
 * monitoring on or off.  Otherwise, do nothing.
 */
int meta_jack_port_request_monitor (jack_port_t *port, int onoff) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * If @ref JackPortCanMonitor is set for this @a port_name, turn input
 * monitoring on or off.  Otherwise, do nothing.
 *
 * @return 0 on success, otherwise a non-zero error code.
 *
 * @see jack_port_name_size()
 */
int meta_jack_port_request_monitor_by_name (jack_client_t *client,
                                       const char *port_name, int onoff) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * If @ref JackPortCanMonitor is set for a port, this function turns
 * on input monitoring if it was off, and turns it off if only one
 * request has been made to turn it on.  Otherwise it does nothing.
 *
 * @return 0 on success, otherwise a non-zero error code
 */
int meta_jack_port_ensure_monitor (jack_port_t *port, int onoff) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @return TRUE if input monitoring has been requested for @a port.
 */
int meta_jack_port_monitoring_input (jack_port_t *port) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Establish a connection between two ports.
 *
 * When a connection exists, data written to the source port will
 * be available to be read at the destination port.
 *
 * @pre The port types must be identical.
 *
 * @pre The @ref JackPortFlags of the @a source_port must include @ref
 * JackPortIsOutput.
 *
 * @pre The @ref JackPortFlags of the @a destination_port must include
 * @ref JackPortIsInput.
 *
 * @return 0 on success, EEXIST if the connection is already made,
 * otherwise a non-zero error code
 */
int meta_jack_connect (jack_client_t *,
                  const char *source_port,
                  const char *destination_port) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Remove a connection between two ports.
 *
 * @pre The port types must be identical.
 *
 * @pre The @ref JackPortFlags of the @a source_port must include @ref
 * JackPortIsOutput.
 *
 * @pre The @ref JackPortFlags of the @a destination_port must include
 * @ref JackPortIsInput.
 *
 * @return 0 on success, otherwise a non-zero error code
 */
int meta_jack_disconnect (jack_client_t *,
                     const char *source_port,
                     const char *destination_port) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Perform the same function as jack_disconnect() using port handles
 * rather than names.  This avoids the name lookup inherent in the
 * name-based version.
 *
 * Clients connecting their own ports are likely to use this function,
 * while generic connection clients (e.g. patchbays) would use
 * jack_disconnect().
 */
int meta_jack_port_disconnect (jack_client_t *, jack_port_t *) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @return the maximum number of characters in a full JACK port name
 * including the final NULL character.  This value is a constant.
 *
 * A port's full name contains the owning client name concatenated
 * with a colon (:) followed by its short name and a NULL
 * character.
 */
int meta_jack_port_name_size(void) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @return the maximum number of characters in a JACK port type name
 * including the final NULL character.  This value is a constant.
 */
int meta_jack_port_type_size(void) JACK_OPTIONAL_WEAK_EXPORT;

/*@}*/

/**
 * @defgroup PortSearching Looking up ports
 * @{
 */

/**
 * @param port_name_pattern A regular expression used to select
 * ports by name.  If NULL or of zero length, no selection based
 * on name will be carried out.
 * @param type_name_pattern A regular expression used to select
 * ports by type.  If NULL or of zero length, no selection based
 * on type will be carried out.
 * @param flags A value used to select ports by their flags.
 * If zero, no selection based on flags will be carried out.
 *
 * @return a NULL-terminated array of ports that match the specified
 * arguments.  The caller is responsible for calling jack_free(3) any
 * non-NULL returned value.
 *
 * @see jack_port_name_size(), jack_port_type_size()
 */
const char ** meta_jack_get_ports (jack_client_t *,
                              const char *port_name_pattern,
                              const char *type_name_pattern,
                              unsigned long flags) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @return address of the jack_port_t named @a port_name.
 *
 * @see jack_port_name_size()
 */
jack_port_t * meta_jack_port_by_name (jack_client_t *, const char *port_name) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @return address of the jack_port_t of a @a port_id.
 */
jack_port_t * meta_jack_port_by_id (jack_client_t *client,
                               jack_port_id_t port_id) JACK_OPTIONAL_WEAK_EXPORT;

/*@}*/

/**
 * @defgroup TimeFunctions Handling time
 * @{
 *
 * JACK time is in units of 'frames', according to the current sample rate.
 * The absolute value of frame times is meaningless, frame times have meaning
 * only relative to each other.
 */

/**
 * @return the estimated time in frames that has passed since the JACK
 * server began the current process cycle.
 */
jack_nframes_t meta_jack_frames_since_cycle_start (const jack_client_t *) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @return the estimated current time in frames.
 * This function is intended for use in other threads (not the process
 * callback).  The return value can be compared with the value of
 * jack_last_frame_time to relate time in other threads to JACK time.
 */
jack_nframes_t meta_jack_frame_time (const jack_client_t *) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @return the precise time at the start of the current process cycle.
 * This function may only be used from the process callback, and can
 * be used to interpret timestamps generated by jack_frame_time() in
 * other threads with respect to the current process cycle.
 *
 * This is the only jack time function that returns exact time:
 * when used during the process callback it always returns the same
 * value (until the next process callback, where it will return
 * that value + nframes, etc).  The return value is guaranteed to be
 * monotonic and linear in this fashion unless an xrun occurs.
 * If an xrun occurs, clients must check this value again, as time
 * may have advanced in a non-linear way (e.g. cycles may have been skipped).
 */
jack_nframes_t meta_jack_last_frame_time (const jack_client_t *client) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @return the estimated time in microseconds of the specified frame time
 */
jack_time_t meta_jack_frames_to_time(const jack_client_t *client, jack_nframes_t) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @return the estimated time in frames for the specified system time.
 */
jack_nframes_t meta_jack_time_to_frames(const jack_client_t *client, jack_time_t) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @return return JACK's current system time in microseconds,
 *         using the JACK clock source.
 *
 * The value returned is guaranteed to be monotonic, but not linear.
 */
jack_time_t meta_jack_get_time() JACK_OPTIONAL_WEAK_EXPORT;

/*@}*/

/**
 * @defgroup ErrorOutput Controlling error/information output
 */
/*@{*/

///**
// * Display JACK error message.
// *
// * Set via jack_set_error_function(), otherwise a JACK-provided
// * default will print @a msg (plus a newline) to stderr.
// *
// * @param msg error message text (no newline at end).
// */
//extern void (*jack_error_callback)(const char *msg) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Set the @ref jack_error_callback for error message display.
 * Set it to NULL to restore default_jack_error_callback function.
 *
 * The JACK library provides two built-in callbacks for this purpose:
 * default_jack_error_callback() and silent_jack_error_callback().
 */
void meta_jack_set_error_function (void (*func)(const char *)) JACK_OPTIONAL_WEAK_EXPORT;

///**
// * Display JACK info message.
// *
// * Set via jack_set_info_function(), otherwise a JACK-provided
// * default will print @a msg (plus a newline) to stdout.
// *
// * @param msg info message text (no newline at end).
// */
//extern void (*jack_info_callback)(const char *msg) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Set the @ref jack_info_callback for info message display.
 * Set it to NULL to restore default_jack_info_callback function.
 *
 * The JACK library provides two built-in callbacks for this purpose:
 * default_jack_info_callback() and silent_jack_info_callback().
 */
void meta_jack_set_info_function (void (*func)(const char *)) JACK_OPTIONAL_WEAK_EXPORT;

/*@}*/

/**
 * The free function to be used on memory returned by jack_port_get_connections,
 * jack_port_get_all_connections and jack_get_ports functions.
 * This is MANDATORY on Windows when otherwise all nasty runtime version related crashes can occur.
 * Developers are strongly encouraged to use this function instead of the standard "free" function in new code.
 *
 */
void meta_jack_free(void* ptr) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * @defgroup TransportControl Transport and Timebase control
 * @{
 */

/**
 * Called by the timebase master to release itself from that
 * responsibility.
 *
 * If the timebase master releases the timebase or leaves the JACK
 * graph for any reason, the JACK engine takes over at the start of
 * the next process cycle.  The transport state does not change.  If
 * rolling, it continues to play, with frame numbers as the only
 * available position information.
 *
 * @see jack_set_timebase_callback
 *
 * @param client the JACK client structure.
 *
 * @return 0 on success, otherwise a non-zero error code.
 */
int  meta_jack_release_timebase (jack_client_t *client) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Register (or unregister) as a slow-sync client, one that cannot
 * respond immediately to transport position changes.
 *
 * The @a sync_callback will be invoked at the first available
 * opportunity after its registration is complete.  If the client is
 * currently active this will be the following process cycle,
 * otherwise it will be the first cycle after calling jack_activate().
 * After that, it runs according to the ::JackSyncCallback rules.
 * Clients that don't set a @a sync_callback are assumed to be ready
 * immediately any time the transport wants to start.
 *
 * @param client the JACK client structure.
 * @param sync_callback is a realtime function that returns TRUE when
 * the client is ready.  Setting @a sync_callback to NULL declares that
 * this client no longer requires slow-sync processing.
 * @param arg an argument for the @a sync_callback function.
 *
 * @return 0 on success, otherwise a non-zero error code.
 */
int  meta_jack_set_sync_callback (jack_client_t *client,
                             JackSyncCallback sync_callback,
                             void *arg) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Set the timeout value for slow-sync clients.
 *
 * This timeout prevents unresponsive slow-sync clients from
 * completely halting the transport mechanism.  The default is two
 * seconds.  When the timeout expires, the transport starts rolling,
 * even if some slow-sync clients are still unready.  The @a
 * sync_callbacks of these clients continue being invoked, giving them
 * a chance to catch up.
 *
 * @see jack_set_sync_callback
 *
 * @param client the JACK client structure.
 * @param timeout is delay (in microseconds) before the timeout expires.
 *
 * @return 0 on success, otherwise a non-zero error code.
 */
int  meta_jack_set_sync_timeout (jack_client_t *client,
                            jack_time_t timeout) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Register as timebase master for the JACK subsystem.
 *
 * The timebase master registers a callback that updates extended
 * position information such as beats or timecode whenever necessary.
 * Without this extended information, there is no need for this
 * function.
 *
 * There is never more than one master at a time.  When a new client
 * takes over, the former @a timebase_callback is no longer called.
 * Taking over the timebase may be done conditionally, so it fails if
 * there was a master already.
 *
 * @param client the JACK client structure.
 * @param conditional non-zero for a conditional request.
 * @param timebase_callback is a realtime function that returns
 * position information.
 * @param arg an argument for the @a timebase_callback function.
 *
 * @return
 *   - 0 on success;
 *   - EBUSY if a conditional request fails because there was already a
 *   timebase master;
 *   - other non-zero error code.
 */
int  meta_jack_set_timebase_callback (jack_client_t *client,
                                 int conditional,
                                 JackTimebaseCallback timebase_callback,
                                 void *arg) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Reposition the transport to a new frame number.
 *
 * May be called at any time by any client.  The new position takes
 * effect in two process cycles.  If there are slow-sync clients and
 * the transport is already rolling, it will enter the
 * ::JackTransportStarting state and begin invoking their @a
 * sync_callbacks until ready.  This function is realtime-safe.
 *
 * @see jack_transport_reposition, jack_set_sync_callback
 *
 * @param client the JACK client structure.
 * @param frame frame number of new transport position.
 *
 * @return 0 if valid request, non-zero otherwise.
 */
int  meta_jack_transport_locate (jack_client_t *client,
                            jack_nframes_t frame) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Query the current transport state and position.
 *
 * This function is realtime-safe, and can be called from any thread.
 * If called from the process thread, @a pos corresponds to the first
 * frame of the current cycle and the state returned is valid for the
 * entire cycle.
 *
 * @param client the JACK client structure.
 * @param pos pointer to structure for returning current transport
 * position; @a pos->valid will show which fields contain valid data.
 * If @a pos is NULL, do not return position information.
 *
 * @return Current transport state.
 */
jack_transport_state_t meta_jack_transport_query (const jack_client_t *client,
                                             jack_position_t *pos) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Return an estimate of the current transport frame,
 * including any time elapsed since the last transport
 * positional update.
 *
 * @param client the JACK client structure
 */
jack_nframes_t meta_jack_get_current_transport_frame (const jack_client_t *client) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Request a new transport position.
 *
 * May be called at any time by any client.  The new position takes
 * effect in two process cycles.  If there are slow-sync clients and
 * the transport is already rolling, it will enter the
 * ::JackTransportStarting state and begin invoking their @a
 * sync_callbacks until ready.  This function is realtime-safe.
 *
 * @see jack_transport_locate, jack_set_sync_callback
 *
 * @param client the JACK client structure.
 * @param pos requested new transport position.
 *
 * @return 0 if valid request, EINVAL if position structure rejected.
 */
int  meta_jack_transport_reposition (jack_client_t *client,
                                jack_position_t *pos) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Start the JACK transport rolling.
 *
 * Any client can make this request at any time.  It takes effect no
 * sooner than the next process cycle, perhaps later if there are
 * slow-sync clients.  This function is realtime-safe.
 *
 * @see jack_set_sync_callback
 *
 * @param client the JACK client structure.
 */
void meta_jack_transport_start (jack_client_t *client) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Stop the JACK transport.
 *
 * Any client can make this request at any time.  It takes effect on
 * the next process cycle.  This function is realtime-safe.
 *
 * @param client the JACK client structure.
 */
void meta_jack_transport_stop (jack_client_t *client) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Gets the current transport info structure (deprecated).
 *
 * @param client the JACK client structure.
 * @param tinfo current transport info structure.  The "valid" field
 * describes which fields contain valid data.
 *
 * @deprecated This is for compatibility with the earlier transport
 * interface.  Use jack_transport_query(), instead.
 *
 * @pre Must be called from the process thread.
 */
void meta_jack_get_transport_info (jack_client_t *client,
                              jack_transport_info_t *tinfo) JACK_OPTIONAL_WEAK_EXPORT;

/**
 * Set the transport info structure (deprecated).
 *
 * @deprecated This function still exists for compatibility with the
 * earlier transport interface, but it does nothing.  Instead, define
 * a ::JackTimebaseCallback.
 */
void meta_jack_set_transport_info (jack_client_t *client,
                              jack_transport_info_t *tinfo) JACK_OPTIONAL_WEAK_EXPORT;

#ifdef __cplusplus
}
#endif

#endif // META_JACK_H
