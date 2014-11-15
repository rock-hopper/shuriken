/*
  This file contains code originally written by "jpo"
  <http://www.juce.com/comment/296820#comment-296820>

  All modifications to jpo's code by Andrew M Taylor <a.m.taylor303@gmail.com>, 2014

  This is free and unencumbered software released into the public domain.
  Please read UNLICENSE for more details, or refer to <http://unlicense.org/>

*/

#include <dlfcn.h>
#include <stdlib.h>
#include <jack/jack.h>
#include <jack/midiport.h>
#include <jack/session.h>
#include <jack/transport.h>

#include <cassert>
#include <iostream>

/* dynamically load libjack and forward all registered calls to libjack
   (similar to what relaytool is trying to do, but more portably..)
*/

using std::cerr;


int libjack_is_present = 0; // public symbol , similar to what relaytool does.
int libjack_session_is_supported = 0;

static void *libjack_handle = 0;

static void __attribute__((constructor)) tryload_libjack()
{
  if (getenv("SKIP_LIBJACK") == 0) { // just in case libjack is causing troubles..
    libjack_handle = dlopen("libjack.so.0", RTLD_LAZY);
  }
  libjack_is_present = (libjack_handle != 0);
  libjack_session_is_supported = (libjack_handle && dlsym(libjack_handle, "jack_set_session_callback") != 0);
}

void *load_jack_function(const char *fn_name) {
  void *fn = 0;
  if (!libjack_handle) {
    std::cerr << "libjack not found, so do not try to load " << fn_name << " ffs !\n";
    return 0;
  }
  fn = dlsym(libjack_handle, fn_name);
  if (!fn) {
    std::cerr << "could not dlsym(" << libjack_handle << "), " << dlerror() << "\n";
  }
  return fn;
}

#define DECL_FUNCTION(return_type, fn_name, arguments_types, arguments) \
  typedef return_type (*fn_name##_ptr_t)arguments_types;                \
  return_type fn_name arguments_types {                                 \
    static fn_name##_ptr_t fn = 0;                                      \
    if (fn == 0) { fn = (fn_name##_ptr_t)load_jack_function(#fn_name); } \
    if (fn) return (*fn)arguments;                                      \
    else return 0;                                                      \
  }

#define DECL_VOID_FUNCTION(fn_name, arguments_types, arguments)         \
  typedef void (*fn_name##_ptr_t)arguments_types;                       \
  void fn_name arguments_types {                                        \
    static fn_name##_ptr_t fn = 0;                                      \
    if (fn == 0) { fn = (fn_name##_ptr_t)load_jack_function(#fn_name); } \
    if (fn) (*fn)arguments;                                             \
  }

DECL_FUNCTION(jack_client_t *, jack_client_open, (const char *client_name, jack_options_t options, jack_status_t *status, ...),
              (client_name, options, status));

// variant with the session UUID..
jack_client_t *jack_client_open_with_uuid(const char *client_name, jack_options_t options, jack_status_t *status, const char *uuid) {
  static jack_client_open_ptr_t fn = 0;
  if (fn == 0) { fn = (jack_client_open_ptr_t)load_jack_function("jack_client_open"); }
  if (fn) return (*fn)(client_name, options, status, uuid);
  else return 0;
}

DECL_FUNCTION(int, jack_client_close, (jack_client_t *client), (client));
DECL_FUNCTION(int, jack_activate, (jack_client_t *client), (client));
DECL_FUNCTION(int, jack_deactivate, (jack_client_t *client), (client));
DECL_FUNCTION(jack_nframes_t, jack_get_buffer_size, (jack_client_t *client), (client));
DECL_FUNCTION(jack_nframes_t, jack_get_sample_rate, (jack_client_t *client), (client));
DECL_VOID_FUNCTION(jack_on_shutdown, (jack_client_t *client, void (*function)(void *arg), void *arg), (client, function, arg));
DECL_FUNCTION(void *, jack_port_get_buffer, (jack_port_t *port, jack_nframes_t nframes), (port, nframes));
DECL_FUNCTION(jack_nframes_t, jack_port_get_total_latency, (jack_client_t *client, jack_port_t *port), (client, port));
DECL_FUNCTION(jack_port_t *, jack_port_register, (jack_client_t *client, const char *port_name, const char *port_type,
                                                  unsigned long flags, unsigned long buffer_size),
              (client, port_name, port_type, flags, buffer_size));
DECL_FUNCTION(int, jack_port_unregister, (jack_client_t *client, jack_port_t *port), (client, port));
DECL_VOID_FUNCTION(jack_set_error_function, (void (*func)(const char *)), (func));
DECL_FUNCTION(int, jack_set_process_callback, (jack_client_t *client, JackProcessCallback process_callback, void *arg), (client, process_callback, arg));
DECL_FUNCTION(int, jack_set_freewheel_callback, (jack_client_t *client, JackFreewheelCallback freewheel_callback, void *arg), (client, freewheel_callback, arg));


DECL_FUNCTION(const char**, jack_get_ports, (jack_client_t *client, const char *port_name_pattern, const char *   type_name_pattern,
                                             unsigned long flags), (client, port_name_pattern, type_name_pattern, flags));
DECL_FUNCTION(int, jack_connect, (jack_client_t *client, const char *source_port, const char *destination_port), (client, source_port, destination_port));
DECL_FUNCTION(const char*, jack_port_name, (const jack_port_t *port), (port));
DECL_FUNCTION(int, jack_set_port_connect_callback, (jack_client_t *client, JackPortConnectCallback connect_callback, void *arg),
              (client, connect_callback, arg));
DECL_FUNCTION(jack_port_t *, jack_port_by_id, (jack_client_t *client, jack_port_id_t port_id), (client, port_id));

DECL_FUNCTION(jack_nframes_t, jack_midi_get_event_count, (void* port_buffer), (port_buffer));
DECL_FUNCTION(int, jack_midi_event_get, (jack_midi_event_t *event, void *port_buffer, jack_nframes_t event_index), (event, port_buffer, event_index));

DECL_FUNCTION(int, jack_session_reply, (jack_client_t *client, jack_session_event_t *event), (client, event));
DECL_FUNCTION(int, jack_set_session_callback, (jack_client_t *client, JackSessionCallback session_callback, void *arg),
              (client, session_callback, arg));
DECL_VOID_FUNCTION(jack_session_event_free, (jack_session_event_t *event), (event));

typedef jack_transport_state_t (*jack_transport_query_ptr_t)(const jack_client_t *client, jack_position_t *pos);
jack_transport_state_t jack_transport_query(const jack_client_t *client, jack_position_t *pos) {
  static jack_transport_query_ptr_t fn = 0;
  if (fn == 0) { fn = (jack_transport_query_ptr_t)load_jack_function("jack_transport_query"); }
  if (fn) return (*fn)(client, pos);
  else return JackTransportStopped;
}
