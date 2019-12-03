#ifndef __TRACECLIENT_THREAD_H__
#define __TRACECLIENT_THREAD_H__

#include "gmthread.h"
#include "gmapp.h"
#include "trace.pb.h"
#include "trace_typedef.h"

#define TIMER_ID_HEARTBEAT 1
#define TIMER_ID_HEARTBEAT_INTERVAL 120000

#define TIMER_ID_CHECK_GNS_REPORT_STATUS 2
#define TIMER_ID_CHECK_GNS_REPORT_STATUS_INTERVAL 60000

//user may extend this struct
typedef struct tagCfgInfo
{
	std::string session_id;
	int p1;
	long p2;
	std::string p3;

	tagCfgInfo(std::string p_session_id, int p_p1, long p_p2, std::string p_p3) :
		session_id(p_session_id), p1(p_p1), p2(p_p2), p3(p_p3) {}
    
    tagCfgInfo() {}

}CfgInfo;

class TraceClientThread : public CGMThread
{
private:
    GM_CONN_INFO* m_pCon;
	bool m_has_report_gns;

protected:
	//Containers of objects suffer from object slicing. So here we use pointers for users to extend CfgInfo
    std::vector<CfgInfo*> m_cfgInfos;
	
public:
	std::vector<std::string> m_ip_port;
	std::vector<std::string> m_gns_name;
	std::string m_trace_server_ip;
	int m_trace_server_port;

public:
    TraceClientThread();
    ~TraceClientThread();
    
    // thread already started.  for initializing something
    virtual int OnThreadStarted();

    // time out function.
    virtual void OnTimer(UINT32 timerid, UINT32 par1, void *par2);

    // get something from local
    virtual void OnLocalMsg(char *pMsg);

    virtual bool ParseNetMsg(int fd, GM_CONN_INFO *pCon, int &readlen);

	virtual void ReleaseConnection(GM_CONN_INFO *p);

	void OnNetMsg(int fd, UINT32 revents, void *handler);

	void CreateConnectionToServer();

	void ClearSessionInCfg(std::string strContent);
    void HandleMsg(int msg_id, int msg_len, std::string strContent, int fd, GM_CONN_INFO *pCon);
    int SendMsg(std::string msg, int msg_id);

    int SetSrvSocketOpts(int socket);
    int CreateProxyConnection();
    int ReportSelfInfo();
	void SendHeartBeatMsg();

	virtual void setConfig(const std::string strContent);
	virtual bool check(const CfgInfo& cfg, const char *checkif);
	void SaveConfig(::pb::CfgInfoReq oCfgPB);
	void TLogIf(const char* filename, int line, const char* funcname, const char* checkif, const char* fmt, ...);
	void TLog(const char* filename, int line, const char* funcname, const char* fmt, ...);
    void TLogSession(const char* filename, int line, const char* funcname, std::string sessionid, const char* fmt, ...);
	int UploadLog(const std::string& session_id, const char* file_name, int line, const char* func_name, const char* fmt, va_list va);
};

extern CGMApp *g_pTracer;

#define TRACE_SESSION(handle, sessionid, fmt, ...) ((TraceClientThread*)(handle->m_Threads[0]))->TLogSession(__FILE__, __LINE__, __FUNCTION__, sessionid, fmt, ##__VA_ARGS__);
#define TRACE(handle, fmt, ...) ((TraceClientThread*)(handle->m_Threads[0]))->TLog(__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);
#define TRACE_IF(handle, checkif, fmt, ...) ((CTraceLog*)(handle->m_Threads[0]))->TLogIf(__FILE__, __LINE__, __FUNCTION__, checkif, fmt, ##__VA_ARGS__);
#define TRACE_COND(handle, b, fmt, ...) if(b){((TraceClientThread*)(handle->m_Threads[0]))->TLog(__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__); }

#endif  //__TRACECLIENT_THREAD_H__

