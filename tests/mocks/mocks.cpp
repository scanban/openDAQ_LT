#include "RTOS.h"
#include "IP.h"
#include "IP_Webserver.h"
#include "IP_WEBSOCKET.h"
#include "streaming_signals.h"
#include "streaming_handler.h"
#include <cstring>

extern "C" {

// RTOS.h mocks
void OS_TASK_Terminate(OS_TASK *pTask) { (void)pTask; }
void OS_Delay(int ms) { (void)ms; }
void OS_MUTEX_Create(OS_MUTEX *pMutex) { (void)pMutex; }
void OS_MUTEX_LockBlocked(OS_MUTEX *pMutex) { (void)pMutex; }
void OS_MUTEX_Unlock(OS_MUTEX *pMutex) { (void)pMutex; }
void OS_MAILBOX_Create(OS_MAILBOX *pMailBox, int sizeof_item, int max_items, void *pBuffer) { (void)pMailBox; (void)sizeof_item; (void)max_items; (void)pBuffer; }
int OS_MAILBOX_Put(OS_MAILBOX *pMailBox, const void *pData) { (void)pMailBox; (void)pData; return 0; }
void OS_MAILBOX_GetPtrBlocked(OS_MAILBOX *pMailBox, void **ppData) { (void)pMailBox; (void)ppData; }
void OS_MAILBOX_Purge(OS_MAILBOX *pMailBox) { (void)pMailBox; }

// IP.h mocks
int IP_TCP_SendAndFree(int socket, IP_PACKET *pPacket) { (void)socket; (void)pPacket; return 0; }
int IP_SOCKET_GetErrorCode(int socket) { (void)socket; return 0; }
int IP_MDNS_SERVER_Start(IP_DNS_SERVER_CONFIG *pConfig) { (void)pConfig; return 0; }
int IP_MDNS_SERVER_Stop(void) { return 0; }
void IP_ExecDelayed(IP_EXEC_DELAYED *pDelayed, IP_DELAYED_FUNC *pfFunc, void *pData, void *pContext, void *pfRemove) { (void)pDelayed; (void)pfFunc; (void)pData; (void)pContext; (void)pfRemove; }
void IP_UDP_ReducePayloadLen(IP_PACKET *pPacket, int Len) { (void)pPacket; (void)Len; }

uint16_t htons(uint16_t hostshort) { return hostshort; }
uint32_t htonl(uint32_t hostlong) { return hostlong; }
int socket(int domain, int type, int protocol) { (void)domain; (void)type; (void)protocol; return 0; }
int bind(int sockfd, const struct sockaddr *addr, uint32_t addrlen) { (void)sockfd; (void)addr; (void)addrlen; return 0; }
int listen(int sockfd, int backlog) { (void)sockfd; (void)backlog; return 0; }
int accept(int sockfd, struct sockaddr *addr, uint32_t *addrlen) { (void)sockfd; (void)addr; (void)addrlen; return 0; }
int closesocket(int sockfd) { (void)sockfd; return 0; }
int send(int sockfd, const void *buf, size_t len, int flags) { (void)sockfd; (void)buf; (void)len; (void)flags; return 0; }

// IP_Webserver.h mocks
char mock_webs_send_buffer[2048];
int mock_webs_send_len = 0;
char mock_webs_copy_buffer[2048];
int mock_webs_copy_len = 0;

void IP_WEBS_SendMem(void *privdata, const void *data, size_t data_len) {
    (void)privdata;
    if (mock_webs_send_len + data_len <= sizeof(mock_webs_send_buffer)) {
        memcpy(mock_webs_send_buffer + mock_webs_send_len, data, data_len);
        mock_webs_send_len += data_len;
    }
}
int IP_WEBS_METHOD_CopyData(IP_WEBS_CONTEXT *pContext, void *buf, size_t ContentLen) {
    (void)pContext;
    int to_copy = ContentLen < (size_t)mock_webs_copy_len ? ContentLen : mock_webs_copy_len;
    memcpy(buf, mock_webs_copy_buffer, to_copy);
    return to_copy;
}
void IP_WEBS_SendHeaderEx(IP_WEBS_OUTPUT *pOutput, const char *pHeader, const char *pContentType, int ConnectionClose) { (void)pOutput; (void)pHeader; (void)pContentType; (void)ConnectionClose; }
void IP_WEBS_Flush(IP_WEBS_OUTPUT *pOutput) { (void)pOutput; }
void IP_WEBS_METHOD_AddHook_SingleMethod(IP_WEBS_METHOD_HOOK *pHook, void *pfFunc, const char *pPath, const char *pMethod) { (void)pHook; (void)pfFunc; (void)pPath; (void)pMethod; }

// IP_WEBSOCKET.h mocks
int IP_WEBSOCKET_GenerateAcceptKey(const char *pKey, size_t KeyLen, char *pBuffer, size_t BufferSize) { (void)pKey; (void)KeyLen; (void)pBuffer; (void)BufferSize; return 0; }
void IP_WEBS_WEBSOCKET_AddHook(IP_WEBS_WEBSOCKET_HOOK *pHook, const IP_WEBS_WEBSOCKET_API *pApi, const char *pURI, const char *pProtocols) { (void)pHook; (void)pApi; (void)pURI; (void)pProtocols; }

// streaming_handler.h mocks
struct streaming_callbacks *streaming_cbs;
int streaming_send_avail(const struct stream *stream, signal_t **signals, int num_signals) { (void)stream; (void)signals; (void)num_signals; return 0; }
int streaming_send_unavail(const struct stream *stream, signal_t **signals, int num_signals) { (void)stream; (void)signals; (void)num_signals; return 0; }
int streaming_send_subscribed(const struct stream *stream, signal_t *signal) { (void)stream; (void)signal; return 0; }
int streaming_send_unsubscribed(const struct stream *stream, signal_t *signal) { (void)stream; (void)signal; return 0; }
int streaming_send_meta_stream(struct stream *stream) { (void)stream; return 0; }
int streaming_send_meta_signal(const struct stream *stream, signal_t *signal, uint64_t valueIndex) { (void)stream; (void)signal; (void)valueIndex; return 0; }

}
