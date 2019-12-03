#ifndef __GM_GLOBAL_H__
#define __GM_GLOBAL_H__

#include <sys/types.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <iostream>
#include <string>
#include <sstream>
#include "tostring.h"

#define NEWARRAY(p,cls,sz) \
{ \
    try \
    { \
        p = new cls[sz]; \
    } \
    catch(...) \
    { \
        if(p) delete[]  p; \
        p = 0; \
    } \
}

#define NEW(p,cls) \
{ \
    try \
    { \
        p = new cls; \
    } \
    catch(...) \
    { \
        if(p) delete  p; \
        p = 0; \
    } \
}

#define DELETEARRAY(p)  {if(p){delete[] p; p = 0;}}
#define DELETE(p)  {if(p){delete p; p = 0;}}
#define CLOSE_FD(fd) {if(fd >= 0) {close(fd);fd = -1;}}

typedef void *(*POSIX_THREAD_FUNC)(void *);


enum RETURN_CODE
{
    CODE_OK,
    CODE_FAILED,
    CODE_QUEUE_FULL,
    CODE_NO_MSG,
    CODE_ERROR_MSG,
    CODE_ERROR_RECEIVER,
    CODE_NEW_FAILED,
    CODE_ERROR_CREATE_THREAD,
    CODE_ERROR_PARAM,
    CODE_ERROR_SYSCALL,
    CODE_ERROR_READ,
    CODE_ERROR_WRITE,
    CODE_ERROR_CREATE_EPOLL,
    CODE_ERROR_SOCKET,
};


int CreateThread(POSIX_THREAD_FUNC func, void *Param, pthread_t *thr_id, long priority, size_t stacksize);
int CheckAndMakeDirs(const char *pszDir, int *pret);
void GetProcessName(std::string &str,std::string &strPath);

#define GM_DEFAULT_STACK_SIZE (1024*1024)
#define GM_DEFAULT_WAIT_SECONDS 3
#define GM_FD_ALLOWED_GAP 50

extern bool g_GM_show_runlog;

#endif

