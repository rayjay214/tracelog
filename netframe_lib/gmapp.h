#ifndef __GM_APP_H__
#define __GM_APP_H__

#include <vector>
#include <string>
#include "Log.h"
#include <sys/epoll.h>
#include "gmthread.h"
#include "gmtypedef.h"

typedef std::vector<CGMThread *> GMThreadVector;

class CGMApp
{
private:

    std::string m_ModuleName;
    int m_LastMaxFDCount;
    int m_LastMinFDCount;
    
    int Init();
public:
    static volatile bool bKeepRunning;
    static log4cxx::LoggerPtr g_pLogger;
    GMThreadVector m_Threads;

    pthread_mutex_t mtx_Check;
    bool CheckCanAccept(UINT32 id, int val);
public:    
    CGMApp();
    virtual ~CGMApp();

    void SetModuleName(std::string strModule)
    {
        m_ModuleName =strModule;
    }

    /*falg can be 0 and EPOLLET, with ListenFD = -1, the thread will not listen on the listen port  */
    int AddThread(CGMThread *pThr, const char *pName, int flag=EPOLLET, UINT32 timer_count = 2);
    int ListenIPPort(std::string strIP, UINT16 usPort, int backlog);
    void Run(std::string strIP, UINT16 usPort, UINT32 fd_count = 100000, int backlog = 10);
    void SetOpenRunLog();
};

#endif

