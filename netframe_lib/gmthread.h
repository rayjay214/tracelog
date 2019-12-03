#ifndef __GM_THREAD_H__
#define __GM_THREAD_H__

#include <string>
#include <list>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "gmtypedef.h"
#include "gmmessage.h"
#include "gmtimer.h"

#define USER_READBUF_DEFAULT_SIZE    10240
#define USER_WRITEBUF_DEFAULT_SIZE   10240
#define MAX_CONNECTIONS_PER_THREAD 100000

#define GM_DISPATCH_MSG(msg, usRecv) m_pApp->m_Threads[usRecv]->PutMsg(msg)


class CGMApp;
class CGMThread;

/*
This is the default connection block, 
使用者应该继承该控制块，以添加自有的控制块相关内容
*/
struct GM_CONN_INFO: public GM_EPDATA_HEAD
{
    unsigned char *read_buf;
    unsigned char *write_buf;
    int read_idx;  //当前读位置
    int read_max;  // read_buf 容量
    int write_idx; //当前写位置
    int write_max;  // write_buf 容量
    volatile int refcount;
    time_t active_time;
    unsigned int msg_count;
    int bWaitClose;
    
    pthread_mutex_t write_mtx;

    std::string peer_ip;
    unsigned short peer_port;
    
    GM_CONN_INFO();
    virtual ~GM_CONN_INFO();
    
    /*
    该函数提供了基本的实现，应用层可以重载并调用它，
    以实现自有的一些Reset操作
    */
    virtual void Reset();

    /*
    该函数用于应用层自定义一些释放资源的操作
    由于实测发现在控制块释放后，仍有原控制块对应的fd被epoll触 发。
    推荐的实现是把控制块用全局列表管理起来，进行分配与回收。(或是成员变量)
    
    否则程序在运行过程中可能异常coredump
    */
    virtual void ReleaseConnectionBlock()
    {
        bWaitClose = 1;
        fd = -1;
    }

    /*
    以下函数不能重载。但提供给外部封装调用。
    比如 开发者可以写一个封装了=等操作的类，并调用这里的AddRef
    */
    void AddRef();
    void ReleaseRef();

    bool InitializeBuf(size_t read_size, size_t write_size);
    int WriteData(int fd, unsigned char *pBuf, size_t OutLen, int iWaitTime = 0);
};


struct GM_CONN_INFO_MGR
{
    //以下为统计数据，可用于确认内存泄漏
    volatile int m_alloccount;
    pthread_mutex_t m_MtxAlloc;
    std::list<GM_CONN_INFO *>  m_freedFdConn;
    pthread_mutex_t m_MtxFD;
    std::map<int, GM_CONN_INFO*>  m_mapFdConn; 

    size_t m_MaxReadBuf;
    size_t m_MaxWriteBuf;
    
    GM_CONN_INFO_MGR();
    ~GM_CONN_INFO_MGR();

    void IncAllocCount();
    void DecAllocCount();

    GM_CONN_INFO* AllocConnection(CGMThread *pTh);
    void FreeConnection(GM_CONN_INFO* p);

    size_t GetFreeListSize();

    GM_CONN_INFO * ReleaseFromMap(int FD);
    void AddToMap(GM_CONN_INFO* pCon);
    void CheckFDExpireTime(int timeOut, CGMThread *pTH);
};


//Every GMThread will open a fifo for getting msg from other thread.
class CGMThread
{
private:
    UINT32 m_uID;
    UINT32 m_UseBalance;
    pthread_t m_ThreadID;
    std::string m_ThrName;
    
    int m_TcpRecvBufSize;
    int m_TcpSndBufSize;
    int m_TcpKeepAlive;
    
    pthread_mutex_t m_MsgsMutex;
    std::vector< char * > m_Msgs;

    struct epoll_event *event_list;
    int m_EP;
    int m_FDCount;
    GM_EPDATA_HEAD m_R;

    //epoll listen flag,  can be 0 or EPOLLET 
    int m_Flag;

    GM_EPDATA_HEAD m_ListenD;
    CGMTimer m_Timers;

#ifdef __SINGLE_THREAD_FOR_VALGRIND
public:
#endif
    static void *Thread(void *thr);
    virtual void Run();
    void DealMsgs();

    friend class CGMApp;
    friend class GM_CONN_INFO_MGR;
    
    int InitizlizeThread(int fd_count);
    int InitizlizeTimer(UINT32 timer_count = GM_MAX_TIME_UNIT);
    
    void SetID(UINT32 taskid)
    {
        m_uID = taskid;
    }
    void SetThrName(const char *pName)
    {
        m_ThrName = pName;
    }
    void SetApp(CGMApp *pApp)
    {
        m_pApp = pApp;
    }
    
    void SetFlag(int flag)
    {
        m_Flag = flag;
    }

    int Exit();
protected:
    CGMApp *m_pApp;

    /*
    如果使用了s_ConnMgr, 则需要对无消息的连接进行超时回收
    timeOut : seconds
    */
    void CheckFDExpireTime(int timeOut);

    /*void *p 如果传递的是ConnectionBlock指针，要注意虚表问题，用&pCon->fd*/
    int AddFDToEpoll(int fd, GM_EPDATA_HEAD *p, int flag = 0);
    int ModFDToEpoll(int fd, GM_EPDATA_HEAD *p, int flag = 0);
    
    int DelFDFromEpoll(int fd);
    int SetConnectSocketOpts(int socket);
    int SetAcceptSocketOpts(int socket);
    int SetupConnection(std::string strIP, UINT16 usPort, GM_CONN_INFO *p);
    int SetTimer(UINT32 timerid, UINT32 par1, void *par2, unsigned long timeout, char tag);
    
    void SetTcpSndBufSize(int size)
    {
        m_TcpSndBufSize = size;
    }
    
    void SetTcpRecvBufSize(int size)
    {
        m_TcpRecvBufSize = size;
    }
    
    void SetTcpKeepAlive()
    {
        m_TcpKeepAlive = 1;
    }

    /*
         connection block中的buffer 大小。 必须大于最大的消息长度。
    */
    void SetRWBufSize(size_t read_size, size_t write_size)
    {
        s_ConnMgr.m_MaxReadBuf = read_size;
        s_ConnMgr.m_MaxWriteBuf = write_size;
    }
    
    char *AllocateMsgToOtherThread(UINT32 size, UINT16 dest, UINT32 msgtype)
    {
        assert (size >= sizeof(GM_MSG_HEAD));
        
        char *pMsg = new char[size];
        GM_MSG_HEAD *pHead = (GM_MSG_HEAD *)pMsg;
        pHead->ulMsgSize = size;
        pHead->ulMsgType = msgtype;
        pHead->usSend = GetID();
        pHead->usRecv = dest;
        return pMsg;
    }

    void SetListenFD(int fd)
    {
        m_ListenD.fd = fd;
    }

    /*
    一般不建议重载AllocateConnection，除非的确需要一些高级特性
    */
    virtual GM_EPDATA_HEAD * AllocateConnection(int fd)
    {
        GM_CONN_INFO *pCon;
        pCon = s_ConnMgr.AllocConnection(this);
        
        pCon->fd = fd;
        pCon->AddRef();

        struct sockaddr_in addr;
        socklen_t addr_len = sizeof(struct sockaddr_in);

        //要看本端ip/port可用 getsockname
        if(0 == getpeername(pCon->fd, (struct sockaddr*)&addr, &addr_len))
        {
            pCon->peer_ip = inet_ntoa(addr.sin_addr);
            pCon->peer_port = ntohs(addr.sin_port);
        }

        //app断连重连要先清理旧连接
        GM_CONN_INFO *pOldCon;
        pOldCon = s_ConnMgr.ReleaseFromMap(fd);
        if(pOldCon) ReleaseConnection(pOldCon);
        s_ConnMgr.AddToMap(pCon);

        /*注意虚表*/
        return (GM_EPDATA_HEAD * )&pCon->fd;
    }


    /*
    一般不建议重载ReleaseConnection，除非的确需要一些高级特性
    */
    virtual void ReleaseConnection(GM_CONN_INFO *p)
    {
        GM_CONN_INFO *pCon = p;

        int FD = pCon->fd;
        if(FD < 0)
        {
            pCon->ReleaseRef();
            return;
        }
        
        DelFDFromEpoll( FD );

        GM_CONN_INFO *pOldCon;
        pOldCon = s_ConnMgr.ReleaseFromMap(FD);
        pCon->bWaitClose = 1;
        pCon->fd = -1;
        pCon->ReleaseRef();

        //最后关闭，以确保其它线程不会在本函数处理过程中拿到同样fd
        close(FD);
    }

    

public:    
    static GM_CONN_INFO_MGR s_ConnMgr;

    CGMThread();
    virtual ~CGMThread();
    
    UINT32 GetID()
    {
        return m_uID;
    }
    
    const std::string &GetThrName()
    {
        return m_ThrName;
    }
    
    int GetFDCount()
    {
        return m_FDCount;
    }
    
    void SetUseBalance()
    {
        m_UseBalance = 1;
    }

    // for other local modules put msg to this thread.
    // instead of using it directly, please use  GM_DISPATCH_MSG macro
    int PutMsg( char * pMsg);


    /*
    如果自定义了ConnectionBlock，则要重载NewConnectionBlock
    */
    virtual GM_CONN_INFO* NewConnectionBlock()
    {
        GM_CONN_INFO* p;
        p = new GM_CONN_INFO();
        return p;
    }

    // thread already started.  for initializing something
    virtual int OnThreadStarted() = 0;

    /*
    永远要记得，OnTimer与OnNetMsg/OnLocalMsg 是在同一个线程中执行
    */
    // time out function.
    virtual void OnTimer(UINT32 timerid, UINT32 par1, void *par2) = 0;

    /*
    永远要记得，OnTimer与OnNetMsg/OnLocalMsg 是在同一个线程中执行
    */
    // get something from local
    virtual void OnLocalMsg(char *pMsg) = 0;

    /*
    永远要记得，OnTimer与OnNetMsg/OnLocalMsg 是在同一个线程中执行

    OnNetMsg  OnMsgOneConDeal  提供了默认的接收消息包并调用解析函数的实现，
    如果要使用不同的实现，可以重载它们
    */
    virtual void OnNetMsg(int fd, UINT32 revents, void *handler);

    /*
        超过data_min 大小的消息，才会调用ParseNetMsg
        返回0成功，返回-1失败并且连接被释放
	*/
    virtual int OnMsgOneConDeal(int fd, UINT32 revents, GM_CONN_INFO *pCon, int data_min);

    /*readlen 表示从pCon->read_buf算起的消息长度，该值如果传错，会影响系统运行*/
    virtual bool ParseNetMsg(int fd, GM_CONN_INFO *pCon, int &readlen) = 0;
};

#endif

