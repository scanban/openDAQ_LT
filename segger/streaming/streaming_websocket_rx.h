#ifndef _STREAMING_WEBSOCKET_RX_H_
#define _STREAMING_WEBSOCKET_RX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "IP.h"

int streaming_rx_callback(long Socket, IP_PACKET *pPacket, int code);

#ifdef __cplusplus
}
#endif

#endif
