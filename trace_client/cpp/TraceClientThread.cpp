#include "TraceClientThread.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

TraceClientThread::TraceClientThread()
{
	m_pCon = NULL;
}

TraceClientThread::~TraceClientThread()
{
}

// thread already started.  for initializing something
int TraceClientThread::OnThreadStarted()
{
    SetTimer(TIMER_ID_HEARTBEAT, NULL, NULL, TIMER_ID_HEARTBEAT_INTERVAL, TIMER_TYPE_PERMENENT);
	SetTimer(TIMER_ID_CHECK_GNS_REPORT_STATUS, NULL, NULL, TIMER_ID_CHECK_GNS_REPORT_STATUS_INTERVAL, TIMER_TYPE_PERMENENT);

    return 0;    
    //return CreateProxyConnection();
}

void TraceClientThread::OnTimer(UINT32 timerid, UINT32 par1, void *par2)
{
    switch(timerid)
    {
    	case TIMER_ID_HEARTBEAT:
        {
            SendHeartBeatMsg();
			break;
        }
		case TIMER_ID_CHECK_GNS_REPORT_STATUS:
		{
			if(!m_has_report_gns)
			{
				ReportSelfInfo();
			}
			break;
		}
    default:
        break;
    }
}

void TraceClientThread::CreateConnectionToServer()
{
	if(!m_pCon)
	{
		MYLOG_WARN(CGMApp::g_pLogger, "connection not exist, reconnect");
		m_pCon = NewConnectionBlock();
		m_pCon->AddRef();
		m_pCon->InitializeBuf(10240, 10240);
		CreateProxyConnection();
	}
}

void TraceClientThread::SendHeartBeatMsg()
{
	if(!m_pCon)
	{
		CreateConnectionToServer();
	}
	
	std::string strPbToSend;
	::pb::HeartBeatMsg oPB;
	oPB.set_reserve("nothing");
	oPB.SerializeToString(&strPbToSend);
	SendMsg(strPbToSend, HEARTBEAT_MSG);
}

// get something from local
void TraceClientThread::OnLocalMsg(char *pMsg)
{
}

void TraceClientThread::ReleaseConnection(GM_CONN_INFO *p)
{
	m_has_report_gns = false;
	CGMThread::ReleaseConnection(p);
}

void TraceClientThread::OnNetMsg(int fd, UINT32 revents, void *handler)
{
    MYLOG_INFO(CGMApp::g_pLogger, "on net msg, fd:%d, revnets:%d", fd, revents);
    GM_CONN_INFO *pCon = (GM_CONN_INFO *)((char *)handler - sizeof(void *));
    assert(&pCon->fd == handler);

    if(fd < 0)
    {
        //already closed, not deleted yet.
        pCon->ReleaseRef();
        return;
    }
    
    if(fd != pCon->fd)
    {
        // normally it should not never happen. but epoll may return freed fd
        MYLOG_WARN(CGMApp::g_pLogger, "Conn:%p fd:%d != %d.",pCon, fd, pCon->fd);
        return;
    }

    pCon->active_time = time(0);

    int ret = OnMsgOneConDeal(fd, revents, pCon, sizeof(MSG_HEAD));
    if(ret < 0)
    {
        //pCon is m_pCon
        assert(pCon == m_pCon);
        m_pCon = NULL;
    }
}

bool TraceClientThread::ParseNetMsg(int fd, GM_CONN_INFO *pCon, int &readlen) 
{
    readlen =0;
    if(sizeof(MSG_HEAD) > (size_t)pCon->read_idx)
    {
        return true;
    }
    
    MSG_HEAD *pHead = (MSG_HEAD *)pCon->read_buf;
    pHead->len = ntohl(pHead->len);
    pHead->id = ntohl(pHead->id);

    if((sizeof(MSG_HEAD) + pHead->len)  > (size_t)pCon->read_idx)
    {
        return true;
    }

    readlen = (sizeof(MSG_HEAD) + pHead->len);
    
    std::string strReaded((char*)(pCon->read_buf + sizeof(MSG_HEAD)), pHead->len);

    MYLOG_DEBUG(CGMApp::g_pLogger, "received %s", strReaded.c_str());

    HandleMsg(pHead->id, pHead->len, strReaded, fd, pCon);
    
    return true;
}

void TraceClientThread::HandleMsg(int msg_id, int msg_len, std::string strContent, int fd, GM_CONN_INFO *pCon)
{
	switch (msg_id)
	{
		case CFG_INFO_REQ:
		{
	        setConfig(strContent);		
			break;
		}
		case HEARTBEAT_MSG:
		{
			//do nothing for the moment
			MYLOG_DEBUG(CGMApp::g_pLogger, "receive heartbeat msg from server");
			break;
		}
		case CLEAR_SESSION_REQ:
		{
			::pb::ClearSessionReq oPB;
			if (!oPB.ParseFromString(strContent))
			{
				MYLOG_ERROR(CGMApp::g_pLogger, "ParseFromString failed!");
				break;
			}
			ClearSessionInCfg(oPB.session_id());		
			break;
		}
		default:
			break;	
	}
}

void TraceClientThread::ClearSessionInCfg(std::string session_id)
{
	bool removeFlag = false;
	for(auto it = m_cfgInfos.begin(); it != m_cfgInfos.end();)
	{
		CfgInfo* pCfg = *it;
		if(pCfg->session_id == session_id)
		{
			MYLOG_INFO(CGMApp::g_pLogger, "ready to remove session %s", session_id.c_str());
			m_cfgInfos.erase(it++);
			removeFlag = true;
			break;
		}
		else
		{
			it++;
		}
	}

	if(!removeFlag)
	{
		MYLOG_INFO(CGMApp::g_pLogger, "find no session %s to remove", session_id.c_str());		
	}
}


int TraceClientThread::SetSrvSocketOpts(int socket)
{
    int flags;

    if((flags = fcntl(socket, F_GETFL, 0)) < 0)
    {
        MYLOG_ERROR(CGMApp::g_pLogger,"socket:%d F_GETFL failed. errno:%d.", socket, errno);
    }

    if(fcntl(socket, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        MYLOG_ERROR(CGMApp::g_pLogger,"socket:%d F_SETFL failed. errno:%d.", socket, errno);
    }

    int TcpRecvBufSize = 1024*1024;
    int TcpSndBufSize = 1024*1024;

    if (setsockopt(socket, SOL_SOCKET, SO_RCVBUF,(const void *) &TcpRecvBufSize, sizeof(int))== -1)
    {
        MYLOG_WARN(CGMApp::g_pLogger,"socket:%d setsockopt(SO_RCVBUF) failed. errno:%d.", socket, errno);
    }

    if (setsockopt(socket, SOL_SOCKET, SO_SNDBUF,(const void *) &TcpSndBufSize, sizeof(int))== -1)
    {
        MYLOG_WARN(CGMApp::g_pLogger,"socket:%d setsockopt(SO_SNDBUF) failed. errno:%d.", socket, errno);
    }

    return 0;
}

int TraceClientThread::CreateProxyConnection()
{
    if(m_pCon->fd >= 0)
    {
        return CODE_OK;
    }

    //choose a GateWay to connect.
    size_t count = 0;
    do
    {
        ++count;
        if(CODE_OK !=  SetupConnection(m_trace_server_ip, m_trace_server_port, m_pCon))
        {
            MYLOG_ERROR(CGMApp::g_pLogger, "connect to srv failed. count:%zu", count);
        }
        else
        {
            m_pCon->active_time = time(0);
            MYLOG_WARN(CGMApp::g_pLogger, "connected to srv fd:%d.", m_pCon->fd);
        }
    }
    while((m_pCon->fd < 0) && (count < 3) );

    if(m_pCon->fd < 0)
    {
        MYLOG_ERROR(CGMApp::g_pLogger, "connect to srv failed. count:%zu", count);
        return CODE_FAILED;
    }
   
    m_pCon->bWaitClose = 0;
 
    SetSrvSocketOpts(m_pCon->fd);
    return CODE_OK;
}

int TraceClientThread::SendMsg(std::string msg, int msg_id)
{
	if(!m_pCon)
	{
		MYLOG_WARN(CGMApp::g_pLogger, "send to server failed, no connection.");
		return -1;
	}

    int count = 0;
    do
    {
        ++count;
        usleep(100);
    }
    while((m_pCon->fd < 0) && (count < 3) );
    
    if(m_pCon->fd < 0)
    {
        MYLOG_WARN(CGMApp::g_pLogger, "connection not ready");
        return 0;
    }


	MSG_HEAD *pHead;
	size_t buf_len;
	size_t sent_len;

	pHead = (MSG_HEAD *)m_pCon->read_buf;
	pHead->id = msg_id;
	pHead->len = msg.size();
	memcpy(m_pCon->read_buf + sizeof(MSG_HEAD), msg.data(), pHead->len);
	buf_len = pHead->len + sizeof(MSG_HEAD);
	pHead->len = htonl(pHead->len);
    pHead->id = htonl(pHead->id);
	
	sent_len = m_pCon->WriteData(m_pCon->fd, m_pCon->read_buf, buf_len, 100);
	if(sent_len == buf_len)
	{
		MYLOG_DEBUG(CGMApp::g_pLogger, "send to %d success", m_pCon->fd);
	}
	else
	{
		MYLOG_WARN(CGMApp::g_pLogger, "send to %d failed, erron:%d, req len:%lu, real len:%lu", m_pCon->fd, errno, buf_len, sent_len);
		ReleaseConnection(m_pCon);
		m_pCon = NULL;
		return -1;
	}

	return 0;
}


//todo send multiple ips and gns_names (need to change pb)
int TraceClientThread::ReportSelfInfo()
{
	CreateConnectionToServer();
		
    if(m_pCon->fd < 0)
        return -1;

    usleep(100000);

	//construct msg to send
	std::string strGnsInfo;
	::pb::GnsInfoReq oPB;
	oPB.add_gns_name(m_gns_name[0]);
	oPB.add_ip_port(m_ip_port[0]);
	oPB.SerializeToString(&strGnsInfo);

	SendMsg(strGnsInfo, GNS_INFO_REQ);

	m_has_report_gns = true;
	
    return CODE_OK;
}

bool TraceClientThread::check(const CfgInfo& cfg, const char *checkif)
{
	return false;
}

void TraceClientThread::setConfig(std::string strContent)
{
    return;
}

void TraceClientThread::SaveConfig(::pb::CfgInfoReq oPB)
{
	//if find same session_id, overwrite, else append
	for(auto it = m_cfgInfos.begin(); it != m_cfgInfos.end();)
	{
		CfgInfo* pCfg = *it;
		if(pCfg->session_id == oPB.session_id())
		{
			it = m_cfgInfos.erase(it);		
		}
		else
		{
			it++;
		}
	}
	CfgInfo* pNewCfg = new CfgInfo(oPB.session_id(), oPB.p1(), oPB.p2(), oPB.p3());
	m_cfgInfos.push_back(pNewCfg);
}

void TraceClientThread::TLogIf(const char* filename, int line, const char* funcname, const char* checkif, const char* fmt, ...)
{
	for(auto it = m_cfgInfos.begin(); it != m_cfgInfos.end(); ++it)
	{
		CfgInfo* pCfg = *it;
		if(check(*pCfg, checkif))
		{
			va_list va;
			va_start(va, fmt);
			UploadLog(pCfg->session_id, filename, line, funcname, fmt, va);
			va_end(va);
		}
	}
}

void TraceClientThread::TLog(const char* filename, int line, const char* funcname, const char* fmt, ...)
{
    for(auto it = m_cfgInfos.begin(); it != m_cfgInfos.end(); ++it)
	{
		CfgInfo* pCfg = *it;
		va_list va;
		va_start(va, fmt);
		UploadLog(pCfg->session_id, filename, line, funcname, fmt, va);
		va_end(va);		
	}
}

void TraceClientThread::TLogSession(const char* filename, int line, const char* funcname, std::string sessionid, const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    UploadLog(sessionid, filename, line, funcname, fmt, va);
    va_end(va);
}


int TraceClientThread::UploadLog(const std::string& session_id, const char* file_name, int line, const char* func_name, const char* fmt, va_list va)
{
	//construct log text with necessary prefix
	char time_buffer[32] = { 0 };
	char time_sec_buffer[64] = { 0 };
	char code_describe[256] = { 0 };
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);
		time_t rawtime;
		rawtime = tv.tv_sec;
		struct tm timeinfo;
		if (localtime_r(&rawtime, &timeinfo))
		{
			strftime(time_buffer, sizeof(time_buffer), "%F %T ", &timeinfo);
			snprintf(time_sec_buffer, sizeof(time_sec_buffer), "%ld ", tv.tv_usec);
		}
		else
		{
			return -1;
		}

		if ((NULL != file_name) && (0 != line) && (NULL != func_name))
		{
			snprintf(code_describe, sizeof(code_describe), "[%s:%d(%s)] ", file_name, line, func_name);
		}
	}
	char content[10240] = {0};
	vsnprintf(content, sizeof(content), fmt, va);
	std::string strLogText = std::string((char *)time_buffer) + std::string((char *)time_sec_buffer) +
		std::string((char*)code_describe) + std::string((char *)content);

	std::string strPbToSend;
	::pb::LogInfo oPB;
	oPB.set_session_id(session_id);
	oPB.set_ip_port(m_ip_port[0]);
	oPB.set_gns_name(m_gns_name[0]);
	oPB.set_log_text(strLogText);
	oPB.SerializeToString(&strPbToSend);
	
	return SendMsg(strPbToSend, LOG_INFO_REQ);
}

