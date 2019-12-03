#ifndef __GM_MESSAGE_H__
#define __GM_MESSAGE_H__

#include "gmglobal.h"
#include "gmtypedef.h"

struct GM_EPDATA_HEAD
{
    volatile int fd;
};

struct GM_MSG_HEAD
{
    UINT32 ulMsgSize;  //include Head
    UINT32 ulMsgType;
    UINT16 usSend;
    UINT16 usRecv;
};

struct GM_MSG_ACK_HEAD: public GM_MSG_HEAD
{
    UINT32 ulReturnCode;
};

#endif

