#ifndef _STREAMING_JSONRPC_H_
#define _STREAMING_JSONRPC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "streaming_config.h"
#include "IP_Webserver.h"

void streaming_jsonrpc_init(char stream_id[9]);
int streaming_jsonrpc_callback(void *pContext, WEBS_OUTPUT *pOutput, const char *sMethod, const char *sAccept,
                                      const char *sContentType, const char *sResource, U32 ContentLen);

#ifdef __cplusplus
}
#endif

#endif
