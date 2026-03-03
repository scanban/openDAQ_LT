#ifndef _IP_WEBSERVER_H_
#define _IP_WEBSERVER_H_

#include "IP.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IP_WEBS_CONTEXT IP_WEBS_CONTEXT;
typedef struct IP_WEBS_OUTPUT IP_WEBS_OUTPUT;

void IP_WEBS_SendMem(void *privdata, const void *data, size_t data_len);
int IP_WEBS_METHOD_CopyData(IP_WEBS_CONTEXT *pContext, void *buf, size_t ContentLen);
void IP_WEBS_SendHeaderEx(IP_WEBS_OUTPUT *pOutput, const char *pHeader, const char *pContentType, int ConnectionClose);
void IP_WEBS_Flush(IP_WEBS_OUTPUT *pOutput);

typedef struct {
    const char *pPath;
    const char *pMethod;
    void *pfFunc;
} IP_WEBS_METHOD_HOOK;

void IP_WEBS_METHOD_AddHook_SingleMethod(IP_WEBS_METHOD_HOOK *pHook, void *pfFunc, const char *pPath, const char *pMethod);

#ifdef __cplusplus
}
#endif

#endif
