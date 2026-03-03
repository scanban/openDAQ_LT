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

#include "streaming_transport.h"

#if STREAMING_TRANSPORT == STREAMING_TRANSPORT_SEGGER
#include "RTOS.h"

transport_socket_t transport_socket_create(void)
{
	return socket(AF_INET, SOCK_STREAM, 0);
}

int transport_socket_bind(transport_socket_t sock)
{
	struct sockaddr_in addr = {
	    .sin_family = AF_INET,
	    .sin_port = htons(STREAMING_TCP_PORT),
	    .sin_addr.s_addr = htonl(ADDR_ANY),
	};
	return bind(sock, (const struct sockaddr *)&addr, sizeof(addr));
}

int transport_socket_listen(transport_socket_t sock)
{
	return listen(sock, 1);
}

transport_socket_t transport_socket_accept(transport_socket_t sock)
{
	return accept(sock, NULL, 0);
}

int transport_socket_send(transport_socket_t sock, const void *pBuffer, size_t NumBytes)
{
	return send(sock, pBuffer, NumBytes, 0);
}

int transport_socket_close(transport_socket_t sock)
{
	return closesocket(sock);
}

int transport_socket_check_error(transport_socket_t sock)
{
	return IP_SOCKET_GetErrorCode(sock);
}

int transport_socket_set_rx_callback(transport_socket_t sock, void *cb)
{
	return setsockopt(sock, SOL_SOCKET, SO_CALLBACK, cb, 0);
}

void transport_delay(int ms)
{
	OS_Delay(ms);
}

void transport_task_terminate(void)
{
	OS_TASK_Terminate(NULL);
}

#else // POSIX

#include <stdio.h>
#include <string.h>

transport_socket_t transport_socket_create(void)
{
	int domain = (STREAMING_PROTOCOL == STREAMING_PROTOCOL_UNIX) ? AF_UNIX : AF_INET;
	return socket(domain, SOCK_STREAM, 0);
}

int transport_socket_bind(transport_socket_t sock)
{
	if (STREAMING_PROTOCOL == STREAMING_PROTOCOL_UNIX) {
		struct sockaddr_un addr;
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strncpy(addr.sun_path, STREAMING_UNIX_PATH, sizeof(addr.sun_path) - 1);
		unlink(STREAMING_UNIX_PATH);
		return bind(sock, (const struct sockaddr *)&addr, sizeof(addr));
	} else {
		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(STREAMING_TCP_PORT);
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		return bind(sock, (const struct sockaddr *)&addr, sizeof(addr));
	}
}

int transport_socket_listen(transport_socket_t sock)
{
	return listen(sock, 1);
}

transport_socket_t transport_socket_accept(transport_socket_t sock)
{
	return accept(sock, NULL, NULL);
}

int transport_socket_send(transport_socket_t sock, const void *pBuffer, size_t NumBytes)
{
	return send(sock, pBuffer, NumBytes, 0);
}

int transport_socket_close(transport_socket_t sock)
{
	return close(sock);
}

int transport_socket_check_error(transport_socket_t sock)
{
	char buf;
	int ret = recv(sock, &buf, 0, MSG_DONTWAIT);
	if (ret == 0) return 1; // Connection closed by peer
	if (ret < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) return 0; // No data, but still connected
		return 1; // Error
	}
	return 0; // Data available, still connected
}

int transport_socket_set_rx_callback(transport_socket_t sock, void *cb)
{
	(void)sock;
	(void)cb;
	// SO_CALLBACK is not supported on POSIX
	return 0;
}

void transport_delay(int ms)
{
	usleep(ms * 1000);
}

void transport_task_terminate(void)
{
	// No task management here, assuming this runs in a thread or process.
}

#endif
