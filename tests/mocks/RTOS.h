#ifndef _RTOS_H_
#define _RTOS_H_

#include "IP.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void OS_TASK_FUNC(void);
typedef struct {
    void *pNext;
    OS_TASK_FUNC *pfFunc;
} OS_TASK;

void OS_TASK_Terminate(OS_TASK *pTask);
void OS_Delay(int ms);

typedef struct {
    U32 v;
} OS_MUTEX;

void OS_MUTEX_Create(OS_MUTEX *pMutex);
void OS_MUTEX_LockBlocked(OS_MUTEX *pMutex);
void OS_MUTEX_Unlock(OS_MUTEX *pMutex);

typedef struct {
    void *pData;
} OS_MAILBOX;

void OS_MAILBOX_Create(OS_MAILBOX *pMailBox, int sizeof_item, int max_items, void *pBuffer);
int OS_MAILBOX_Put(OS_MAILBOX *pMailBox, const void *pData);
void OS_MAILBOX_GetPtrBlocked(OS_MAILBOX *pMailBox, void **ppData);
int OS_MAILBOX_GetPtr(OS_MAILBOX *pMailBox, void **ppData);
void OS_MAILBOX_Purge(OS_MAILBOX *pMailBox);

#ifdef __cplusplus
}
#endif

#endif
