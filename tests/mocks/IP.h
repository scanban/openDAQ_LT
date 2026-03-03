#ifndef _IP_H_
#define _IP_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t U32;
typedef int32_t I32;
typedef uint16_t U16;
typedef int16_t I16;
typedef uint8_t U8;

typedef struct IP_PACKET {
    U8 *pData;
} IP_PACKET;

#define IP_OK 0
#define IP_OK_KEEP_PACKET 1

int IP_TCP_SendAndFree(int socket, IP_PACKET *pPacket);
int IP_SOCKET_GetErrorCode(int socket);

typedef struct {
    const char *Type;
    int Flags;
    int TTL;
    const void *Config;
} IP_DNS_SERVER_SD_CONFIG;

#define IP_DNS_SERVER_TYPE_PTR "PTR"
#define IP_DNS_SERVER_TYPE_SRV "SRV"
#define IP_DNS_SERVER_TYPE_A   "A"
#define IP_DNS_SERVER_TYPE_TXT "TXT"
#define IP_DNS_SERVER_FLAG_FLUSH 1

typedef struct {
    const char *pName;
    int TTL;
    int NumConfigs;
    const IP_DNS_SERVER_SD_CONFIG *pConfigs;
} IP_DNS_SERVER_CONFIG;

int IP_MDNS_SERVER_Start(IP_DNS_SERVER_CONFIG *pConfig);
int IP_MDNS_SERVER_Stop(void);

typedef void IP_DELAYED_FUNC(void *pData, void *pContext);
typedef struct {
    void *pNext;
    IP_DELAYED_FUNC *pfFunc;
    void *pData;
    void *pContext;
    void *pfRemove;
} IP_EXEC_DELAYED;

void IP_ExecDelayed(IP_EXEC_DELAYED *pDelayed, IP_DELAYED_FUNC *pfFunc, void *pData, void *pContext, void *pfRemove);

void IP_UDP_ReducePayloadLen(IP_PACKET *pPacket, int Len);

// Standard socket mocks
#define AF_INET 2
#define SOCK_STREAM 1
#define ADDR_ANY 0

struct in_addr {
    uint32_t s_addr;
};

struct sockaddr_in {
    short sin_family;
    unsigned short sin_port;
    struct in_addr sin_addr;
    char sin_zero[8];
};

struct sockaddr {
    unsigned short sa_family;
    char sa_data[14];
};

uint16_t htons(uint16_t hostshort);
uint32_t htonl(uint32_t hostlong);

int socket(int domain, int type, int protocol);
int bind(int sockfd, const struct sockaddr *addr, uint32_t addrlen);
int listen(int sockfd, int backlog);
int accept(int sockfd, struct sockaddr *addr, uint32_t *addrlen);
int closesocket(int sockfd);
int send(int sockfd, const void *buf, size_t len, int flags);

#ifdef __cplusplus
}
#endif

#endif
