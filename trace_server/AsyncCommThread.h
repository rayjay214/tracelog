#ifndef __COM_THREAD_H__
#define __COM_THREAD_H__

#include "gmthread.h"
#include "stdio.h"
#include <map>
#include "trace_typedef.h"
#include "Log.h"

class AsyncCommThread : public CGMThread
{
public:
    AsyncCommThread();
    ~AsyncCommThread();

    // thread already started.  for initializing something
    virtual int OnThreadStarted();

    // time out function.
    virtual void OnTimer(UINT32 timerid, UINT32 par1, void *par2);

    // get something from local
    virtual void OnLocalMsg(char *pMsg);

    virtual bool ParseNetMsg(int fd, GM_CONN_INFO *pCon, int &leftlen);

	virtual void ReleaseConnection(GM_CONN_INFO *p);
	virtual GM_EPDATA_HEAD * AllocateConnection(int fd);

	void CheckSessionExpireTime(int timeoutInSeconds);
	void SendClearSessionReq(int fd, std::string session_id);

    void HandleMsg(int msg_id, int msg_len, std::string strContent, int fd, GM_CONN_INFO *pCon);
	void UpdateClientInfo(std::string strContent, int fd);
	void DumpClientInfo();
    void SendCfgInfo2Clients(std::string session_id, std::string match, int match_type);
	void LoginLogic(std::string strContent, GM_CONN_INFO *pConn);
    void SetConfig(std::string strContent, GM_CONN_INFO *pConn);
	void SaveLog(std::string strContent, GM_CONN_INFO *pConn);
	void GetLog(std::string strContent, GM_CONN_INFO *pConn);
	void SendCfg2SingleClient(CFG oCfg, std::string sesison_id, int fd);
	int SendMsg(std::string msg, int fd, int msg_id);
	void SendBackHeartBeat(std::string strContent, GM_CONN_INFO *pConn);
};

extern std::map<int, CLIENT_INFO> g_map_fd_cli_info;
extern std::map<std::string, SESSION_INFO> g_map_session_info;
extern log4cxx::LoggerPtr g_logger;
extern std::string g_pass_encrypt;


#endif

