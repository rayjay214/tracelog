#ifndef __COM_THREAD_H__
#define __COM_THREAD_H__

#include "gmthread.h"
#include "stdio.h"
#include <map>
#include "fastcgi.h"
#include "type.h"
#include "Log.h"
#include "trace_typedef.h"

using namespace muduo::net;

/*
    也可不继承，直接使用GM_CONN_INFO
    参考example_client实现
*/
class SelfConn : public GM_CONN_INFO
{
public:
    FastCgiCodec* m_pCodec;
	time_t m_createTime;
	
public:
    SelfConn() : GM_CONN_INFO()
    {
    	m_pCodec = new FastCgiCodec();
		m_createTime = time(0);
    }

    virtual ~SelfConn()
    {
    }

    /*如果重载了，要调用父类的相应函数*/
    virtual void Reset()
    {
        GM_CONN_INFO::Reset();
		m_pCodec->reqBeginTime_ = 0;
		m_pCodec->gotRequest_ = false;
		m_pCodec->stdin_.retrieveAll();
      	m_pCodec->paramsStream_.retrieveAll();
      	m_pCodec->params_.clear();
    }
    
    /*如果重载了，要调用父类的相应函数*/
    virtual void ReleaseConnectionBlock()
    {
        GM_CONN_INFO::ReleaseConnectionBlock();
		m_createTime = 0;
		m_pCodec->reqBeginTime_ = 0;
		m_pCodec->gotRequest_ = false;
		m_pCodec->stdin_.retrieveAll();
      	m_pCodec->paramsStream_.retrieveAll();
      	m_pCodec->params_.clear();
    }    
};

class AsyncCommThread : public CGMThread
{
private:
	GM_CONN_INFO *m_pCon;  //作为客户端的时候使用
	
public:
    AsyncCommThread();
    ~AsyncCommThread();

    // thread already started.  for initializing something
    virtual int OnThreadStarted();

    // time out function.
    virtual void OnTimer(UINT32 timerid, UINT32 par1, void *par2);
    
    virtual void OnNetMsg(int fd, UINT32 revents, void *handler);

    // get something from local
    virtual void OnLocalMsg(char *pMsg);

    virtual bool ParseNetMsg(int fd, GM_CONN_INFO *pCon, int &leftlen);

	virtual GM_CONN_INFO* NewConnectionBlock()
    {
        GM_CONN_INFO* p;
        p = new SelfConn();
        return p;
    }

    void onRequest(SelfConn *pSelfCon);	
    void respond(std::string &out, Buffer* response);
    void endStdout(Buffer* buf);
    void endRequest(Buffer* buf);
    bool onBeginRequest(RecordHeader* pHead, const char* pContent, size_t length, GM_CONN_INFO *pCon);
    void onStdin(const char* pContent, uint16_t length, GM_CONN_INFO *pCon);
    bool onParams(const char* pContent, uint16_t length, GM_CONN_INFO *pCon);
    bool parseAllParams(GM_CONN_INFO *pCon);
    uint32_t readLen(GM_CONN_INFO *pCon);


    int CreateProxyConnection(std::string serverIp, int serverPort);
	void CreateConnectionToServer();
    int SetSrvSocketOpts(int socket);
	void HandleMsg(MSG_HEAD *pHead, std::string strContent);
	void LoginResp(std::string strContent);
    void LoginReq(SelfConn *pSelfCon, SSMap qmap);
	void CheckFCGIOvertimeReq(int timeoutInSeconds);
	void ResetConnStatus(SelfConn* pConn);
	int SendMsg(std::string msg, int msg_id);
	bool CheckConn(int fd, time_t check_time);
	void SetConfigResp(std::string strContent);
    void GetLogResp(std::string strContent);
    void SetConfig(SelfConn *pSelfCon, SSMap qmap);
    void GetLog(SelfConn *pSelfCon, SSMap qmap);
    bool HandleTraceServerMsg(GM_CONN_INFO *pCon, int &readlen);
    bool HandleNginxMsg(RecordHeader *pHead, GM_CONN_INFO *pCon, int &readlen);
    void SendHeartBeatMsg();
	
};

extern log4cxx::LoggerPtr g_logger;

#endif

