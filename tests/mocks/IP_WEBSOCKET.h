#ifndef _IP_WEBSOCKET_H_
#define _IP_WEBSOCKET_H_

#include "IP.h"

#ifdef __cplusplus
extern "C" {
#endif

int IP_WEBSOCKET_GenerateAcceptKey(const char *pKey, size_t KeyLen, char *pBuffer, size_t BufferSize);

#define IP_WEBSOCKET_FRAME_TYPE_CONTINUE 0
#define IP_WEBSOCKET_FRAME_TYPE_TEXT     1
#define IP_WEBSOCKET_FRAME_TYPE_BINARY   2
#define IP_WEBSOCKET_FRAME_TYPE_CLOSE    8
#define IP_WEBSOCKET_FRAME_TYPE_PING     9
#define IP_WEBSOCKET_FRAME_TYPE_PONG     10

#define IP_WEBSOCKET_CLOSE_CODE_ABNORMAL_CLOSURE 1006

typedef int (*IP_WEBSOCKET_GEN_ACCEPT_KEY_FUNC)(const char *pKey, size_t KeyLen, char *pBuffer, size_t BufferSize);
typedef void (*IP_WEBSOCKET_ON_DATA_FUNC)(long Socket, IP_PACKET *pPacket, int code);

typedef struct {
    IP_WEBSOCKET_GEN_ACCEPT_KEY_FUNC pfGenerateAcceptKey;
    IP_WEBSOCKET_ON_DATA_FUNC        pfOnData;
} IP_WEBS_WEBSOCKET_API;

typedef struct {
    const char *pURI;
    const char *pProtocols;
} IP_WEBS_WEBSOCKET_HOOK;

void IP_WEBS_WEBSOCKET_AddHook(IP_WEBS_WEBSOCKET_HOOK *pHook, const IP_WEBS_WEBSOCKET_API *pApi, const char *pURI, const char *pProtocols);

#ifdef __cplusplus
}
#endif

#endif
