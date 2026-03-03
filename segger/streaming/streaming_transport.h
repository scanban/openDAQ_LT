/*
 * Copyright (C) 2023 openDAQ
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _STREAMING_TRANSPORT_H_
#define _STREAMING_TRANSPORT_H_

#include <stdint.h>
#include <stddef.h>
#include "streaming_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#if STREAMING_TRANSPORT == STREAMING_TRANSPORT_SEGGER
	#include "IP.h"
	typedef int transport_socket_t;
	#define TRANSPORT_INVALID_SOCKET (-1)
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <sys/un.h>
	#include <unistd.h>
	#include <errno.h>
	#include <fcntl.h>
	typedef int transport_socket_t;
	#define TRANSPORT_INVALID_SOCKET (-1)
#endif

/**
 * Creates a server socket based on the configured protocol.
 * @return: handle to the created socket or TRANSPORT_INVALID_SOCKET on error.
 */
transport_socket_t transport_socket_create(void);

/**
 * Binds the socket to the configured address (TCP port or UNIX path).
 * @param sock: socket handle.
 * @return: 0 on success, <0 on error.
 */
int transport_socket_bind(transport_socket_t sock);

/**
 * Listens for incoming connections.
 * @param sock: socket handle.
 * @return: 0 on success, <0 on error.
 */
int transport_socket_listen(transport_socket_t sock);

/**
 * Accepts an incoming connection.
 * @param sock: server socket handle.
 * @return: handle to the accepted connection socket or TRANSPORT_INVALID_SOCKET on error.
 */
transport_socket_t transport_socket_accept(transport_socket_t sock);

/**
 * Sends data over the socket.
 * @param sock: socket handle.
 * @param pBuffer: buffer to send.
 * @param NumBytes: number of bytes to send.
 * @return: number of bytes sent or <0 on error.
 */
int transport_socket_send(transport_socket_t sock, const void *pBuffer, size_t NumBytes);

/**
 * Closes the socket.
 * @param sock: socket handle.
 * @return: 0 on success, <0 on error.
 */
int transport_socket_close(transport_socket_t sock);

/**
 * Checks if the socket is still connected and doesn't have an error.
 * @param sock: socket handle.
 * @return: 0 if OK, non-zero if error or peer closed.
 */
int transport_socket_check_error(transport_socket_t sock);

/**
 * Sets a receive callback for the socket (primarily for SEGGER emNet).
 * @param sock: socket handle.
 * @param cb: callback function pointer.
 * @return: 0 on success, <0 on error.
 */
int transport_socket_set_rx_callback(transport_socket_t sock, void *cb);

/**
 * Delay execution for a number of milliseconds.
 * Abstracts OS-specific delay functions.
 * @param ms: milliseconds to delay.
 */
void transport_delay(int ms);

/**
 * Terminate the current task.
 * Abstracts OS-specific task termination.
 */
void transport_task_terminate(void);

#ifdef __cplusplus
}
#endif

#endif
