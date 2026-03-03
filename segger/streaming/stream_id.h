#ifndef _STREAMING_ID_H_
#define _STREAMING_ID_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "streaming_transport.h"

#define NUM_STREAMS_MAX 1

extern struct stream single_stream;
typedef int stream_send(const struct stream *s, const char *pBuffer, size_t NumBytes);
typedef int stream_send_packet(const struct stream *s, void *p);

struct stream {
	stream_send *stream;
	stream_send_packet *streamp;
	transport_socket_t socket_handle;
	const char *id;
};

struct stream *stream_malloc(transport_socket_t socket, const char *id);
void stream_free(struct stream *stream);
void streaming_streams_init(void);

#ifdef __cplusplus
}
#endif

#endif
