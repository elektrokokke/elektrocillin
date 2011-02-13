#ifndef META_JACK_TYPES_H
#define META_JACK_TYPES_H

#include <jack/types.h>

/**
 *  meta_jack_port_t is an opaque type.  You may only access it using the
 *  API provided.
 */
typedef struct _meta_jack_port meta_jack_port_t;

/**
 *  meta_jack_client_t is an opaque type.  You may only access it using the
 *  API provided.
 */
typedef struct _meta_jack_client meta_jack_client_t;

#endif // META_JACK_TYPES_H
