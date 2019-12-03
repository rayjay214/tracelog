#include "AsyncCommThread.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include "gmapp.h"
#include "trace_typedef.h"
#include "trace.pb.h"
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include "helper.h"

#define TIMER_ID_CHECK_FD 1
#define TIMER_CHECK_FD_INTERVAL 30000

#define TIMER_ID_CHECK_SESSION 2
#define TIMER_ID_CHECK_SESSION_INTERVAL 300000

#define CFG_GNS 1
#define CFG_IP_PORT 2

AsyncCommThread::AsyncCommThread()
{
}

AsyncCommThread::~AsyncCommThread()
{
}

void AsyncCommThread::ReleaseConnection(GM_CONN_INFO *p)
{
	auto it = g_map_fd_cli_info.find((int)p->fd);
	if(it != g_map_fd_cli_info.end())
	{
		MYLOG_INFO(g_logger, "ready to remove client, fd:%d", p->fd);
		g_map_fd_cli_info.erase(it);
	}

	CGMThread::ReleaseConnection(p);
}

GM_EPDATA_HEAD * AsyncCommThread::AllocateConnection(int fd)
{
	auto it = g_map_fd_cli_info.find(fd);
	if(it != g_map_fd_cli_info.end())
	{
		MYLOG_INFO(g_logger, "ready to remove client, fd:%d", fd);
		g_map_fd_cli_info.erase(it);
	}
	return CGMThread::AllocateConnection(fd);
}

// thread already started.  for initializing something
int AsyncCommThread::OnThreadStarted()
{
    SetTimer(TIMER_ID_CHECK_FD, 0, 0, TIMER_CHECK_FD_INTERVAL, TIMER_TYPE_PERMENENT);
	SetTimer(TIMER_ID_CHECK_SESSION, 0, 0, TIMER_ID_CHECK_SESSION_INTERVAL, TIMER_TYPE_PERMENENT);
    // return none-zero will cause initialization failed.
    return 0;
}

// time out function.
void AsyncCommThread::OnTimer(UINT32 timerid, UINT32 par1, void *par2)
{
    switch(timerid)
    {
    /*gm框架中没有包含 fd检测，仅提供了CheckFDExpireTime函数*/
    case TIMER_ID_CHECK_FD:
        CheckFDExpireTime(12000);
        break;
	case TIMER_ID_CHECK_SESSION:
		CheckSessionExpireTime(g_conf.sessionExpireSeconds);  
		break;
    default:
        break;
    }
}

void AsyncCommThread::CheckSessionExpireTime(int timeoutInSeconds)
{
	time_t now = time(0);
	for(auto it = g_map_session_info.begin(); it != g_map_session_info.end();)
	{
		SESSION_INFO oSession = it->second;
		if(now - oSession.active_time > timeoutInSeconds)
		{
			MYLOG_INFO(g_logger, "session %s expires, ready to remove it", it->first.c_str());
			//inform all related clients to remove their local session
			for(auto itCfg = oSession.cfgs.begin(); itCfg != oSession.cfgs.end(); itCfg++)
			{
				CFG oCfg = *itCfg;
				if(!oCfg.ip_port.empty())
				{
					int fd = GetFdByIPPort(oCfg.ip_port);
					if(fd != -1)
					{
						SendClearSessionReq(fd, it->first);					
					}	
				}
				else
				{
					std::set<int> set_fd = GetFdByGns(oCfg.gns_name);
					for(auto itFd = set_fd.begin(); itFd != set_fd.end(); itFd++)
					{
						SendClearSessionReq(*itFd, it->first);			
					}
				}
			}
			g_map_session_info.erase(it++);
		}
		else
		{
			it++;
		}
	}
}

void AsyncCommThread::SendClearSessionReq(int fd, std::string session_id)
{
	//make pb
	std::string strPbToSend;
	::pb::ClearSessionReq oPB;
	oPB.set_session_id(session_id);
	oPB.SerializeToString(&strPbToSend);
	SendMsg(strPbToSend, fd, CLEAR_SESSION_REQ);
}

// get something from local
void AsyncCommThread::OnLocalMsg(char *pMsg)
{
}

bool AsyncCommThread::ParseNetMsg(int fd, GM_CONN_INFO *pCon, int &readlen) 
{
    readlen = 0;
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
	
	HandleMsg(pHead->id, pHead->len, strReaded, fd, pCon);

    return true;
}

void AsyncCommThread::DumpClientInfo()
{
	for (auto it = g_map_fd_cli_info.begin(); it != g_map_fd_cli_info.end(); ++it)
	{
		int fd = it->first;
		CLIENT_INFO oInfo = it->second;
		MYLOG_INFO(g_logger, "fd:%d", fd);
		for(auto itGns = oInfo.gns_names.begin(); itGns != oInfo.gns_names.end(); itGns++)
		{
			MYLOG_INFO(g_logger, "gns->%s", (*itGns).c_str());	
		}
		for(auto itIpPort = oInfo.ip_ports.begin(); itIpPort != oInfo.ip_ports.end(); itIpPort++)
		{
			MYLOG_INFO(g_logger, "addr->%s", (*itIpPort).c_str());	
		}	
	}
}

//called whenever app restart or connection reconstructed
void AsyncCommThread::UpdateClientInfo(std::string strContent, int fd)
{
	::pb::GnsInfoReq oPB;
	if (!oPB.ParseFromString(strContent))
	{
		MYLOG_ERROR(g_logger, "ParseFromString failed!");
		return;
	}

	CLIENT_INFO oInfo;
	for(int i = 0; i < oPB.gns_name_size(); i++)
	{
		std::string gns_name = oPB.gns_name(i);
		oInfo.gns_names.push_back(gns_name);
	}
	for(int i = 0; i < oPB.ip_port_size(); i++)
	{
		std::string ip_port = oPB.ip_port(i);
		oInfo.ip_ports.push_back(ip_port);
	}
	
	g_map_fd_cli_info[fd] = oInfo;

	DumpClientInfo();

	//check cfg
	//even n*n*n, the value of n will be really small
	for(auto itSession = g_map_session_info.begin(); itSession != g_map_session_info.end(); itSession++)
	{
		SESSION_INFO oSession = itSession->second;
		for(auto itCfg = oSession.cfgs.begin(); itCfg != oSession.cfgs.end(); itCfg++)
		{
			CFG oCfg = *itCfg;
			for(auto it = oInfo.gns_names.begin(); it != oInfo.gns_names.end(); it++)
			{
				if(oCfg.gns_name == *it)
				{
					SendCfg2SingleClient(oCfg, itSession->first, fd);
				}
			}
		}
	}
	
}

void AsyncCommThread::HandleMsg(int msg_id, int msg_len, std::string strContent, int fd, GM_CONN_INFO *pCon)
{
	switch (msg_id)
	{
		case GNS_INFO_REQ:
		{
	        UpdateClientInfo(strContent, fd);		
			break;
		}
		case LOGIN_REQ:
		{
			LoginLogic(strContent, pCon);
			break;
		}
		case SET_CONF_REQ:
		{
			SetConfig(strContent, pCon);
			break;
		}
		case LOG_INFO_REQ:
		{
			SaveLog(strContent, pCon);
			break;
		}
		case GET_LOG_REQ:
		{
			GetLog(strContent, pCon);
			break;
		}
		case HEARTBEAT_MSG:
		{
			SendBackHeartBeat(strContent, pCon);
			break;
		}
		default:
			MYLOG_WARN(g_logger, "Unknown Msg, msg_id %d", msg_id);
			break;	
	}
}

void AsyncCommThread::SendBackHeartBeat(std::string strContent, GM_CONN_INFO *pConn)
{
	//todo extract useful information from strContent 
	MYLOG_DEBUG(g_logger, "receive heartbeat from %d", pConn->fd);
	std::string strPbToSend;
	::pb::HeartBeatMsg oPB;
	oPB.set_reserve("nothing");
	oPB.SerializeToString(&strPbToSend);
	SendMsg(strPbToSend, pConn->fd, HEARTBEAT_MSG);
}

void AsyncCommThread::LoginLogic(std::string strContent, GM_CONN_INFO *pConn)
{
	::pb::LoginReq oSrcPB;
	if (!oSrcPB.ParseFromString(strContent))
	{
		MYLOG_ERROR(g_logger, "ParseFromString failed!");
		return;
	}

	std::string ret_msg;
	int ret = 0;
	std::string session_id;
	if(oSrcPB.password() != g_pass_encrypt)
    {
        ret_msg = "password error";
        MYLOG_ERROR(g_logger, "[%s]!=[%s]", oSrcPB.password().c_str(), g_pass_encrypt.c_str());
        ret = ERROR_PWD_NOT_MATCH;
    }

	if(ret == 0)
	{
		ret_msg = "OK";
		boost::uuids::uuid uuid = boost::uuids::random_generator()();
	    session_id = boost::uuids::to_string(uuid);
		SESSION_INFO oInfo;
		oInfo.active_time = time(0);
		g_map_session_info.insert(std::pair<std::string, SESSION_INFO>(session_id, oInfo));	
	}
    
	//make res pb
	std::string strPbToSend;
	::pb::LoginResp oPB;
	oPB.set_session_id(session_id);
	oPB.set_fd(oSrcPB.fd());
	oPB.set_req_begin_time(oSrcPB.req_begin_time());
	oPB.set_ret(ret);
	oPB.set_ret_msg(ret_msg);
	oPB.SerializeToString(&strPbToSend);

	SendMsg(strPbToSend, pConn->fd, LOGIN_RESP);
}

//if the browser set multiple gns at the same time, this method should be invoked multiple times
//once per gns_name/ip_port
void AsyncCommThread::SetConfig(std::string strContent, GM_CONN_INFO *pConn)
{
	int ret = 0;
	std::string ret_msg;
	
	::pb::CfgInfoReq oPB;
	if (!oPB.ParseFromString(strContent))
	{
		MYLOG_ERROR(g_logger, "ParseFromString failed!");
		return;
	}

	do 
	{
		auto it = g_map_session_info.find(oPB.session_id());
		if(it == g_map_session_info.end())
		{
			ret_msg = "session not found";
			ret = ERROR_SESSION_NOT_FOUND;
			break;
		}
		SESSION_INFO oInfo = it->second;
		oInfo.active_time = time(0);
		//if gns/ip_port corresponding config exists, overwrite it, else append to cfg vector
		for(auto it = oInfo.cfgs.begin(); it != oInfo.cfgs.end();)
		{
			CFG oCfg = *it;
			if(!oPB.ip_port().empty() && oCfg.ip_port == oPB.ip_port())
			{
				it = oInfo.cfgs.erase(it);
			}
			else if(!oPB.gns_name().empty() && oCfg.gns_name == oPB.gns_name())
			{
				it = oInfo.cfgs.erase(it);		
			}
			else
			{
				it++;
			}
		}
		CFG oNewCfg;
		oNewCfg.gns_name = oPB.gns_name();
		oNewCfg.p1 = oPB.p1();
		oNewCfg.p2 = oPB.p2();
		oNewCfg.p3 = oPB.p3();
		oNewCfg.ip_port = oPB.ip_port();
        oNewCfg.log_idx = 0;
		oInfo.cfgs.push_back(oNewCfg);
		g_map_session_info[oPB.session_id()] = oInfo;
		if(!oPB.ip_port().empty())
		{
			SendCfgInfo2Clients(oPB.session_id(), oPB.ip_port(), CFG_IP_PORT);			
		}
		else
		{
			SendCfgInfo2Clients(oPB.session_id(), oPB.gns_name(), CFG_GNS);	
		}
	}while(0);

	//send response to cgi
	//makd resp pb
	std::string strPbToSend;
	::pb::CfgInfoResp oRespPB;
	oRespPB.set_ret(ret);
	oRespPB.set_ret_msg(ret_msg);
	oRespPB.set_fd(oPB.fd());
	oRespPB.set_req_begin_time(oPB.req_begin_time());
	oRespPB.SerializeToString(&strPbToSend);

	SendMsg(strPbToSend, pConn->fd, SET_CONF_RESP);
}

void AsyncCommThread::SaveLog(std::string strContent, GM_CONN_INFO *pConn)
{
	std::string ret_msg;
	
	::pb::LogInfo oPB;
	if (!oPB.ParseFromString(strContent))
	{
		MYLOG_ERROR(g_logger, "ParseFromString failed!");
		return;
	}

	auto itSession = g_map_session_info.find(oPB.session_id());
	if(itSession == g_map_session_info.end())
	{
		//clear session in client
		MYLOG_WARN(g_logger, "find no session %s", oPB.session_id().c_str());
		SendClearSessionReq(pConn->fd, oPB.session_id());
		return;
	}

	SESSION_INFO &oInfo = itSession->second;
    for(int i = 0; i < (int)oInfo.cfgs.size(); i++)
    {
        CFG &oCfg = oInfo.cfgs.at(i);
		if(oCfg.ip_port == oPB.ip_port())
		{
			MYLOG_INFO(g_logger, "receive info from %s, %s, text %s", 
				oPB.gns_name().c_str(), oPB.ip_port().c_str(), oPB.log_text().c_str());			
			if(oCfg.logs.size() >= (size_t)g_conf.maxPerserveLogs)
			{
				//remove logs which has already been got (before log_idx)
				oCfg.logs.erase(oCfg.logs.begin(), oCfg.logs.begin() + oCfg.log_idx);
				oCfg.log_idx = 0;
				MYLOG_INFO(g_logger, "after erase, size is %lu", oCfg.logs.size());
				if(oCfg.logs.size() >= (size_t)g_conf.maxPerserveLogs)
				{
					MYLOG_WARN(g_logger, "overload, will not receive any logs");
					break;
				}
			}
					
			//add gns_name ip_port to text
			std::string text = oPB.gns_name() + " " + oPB.ip_port() + "  " + oPB.log_text();
			oCfg.logs.push_back(text);
		}

		if(oCfg.gns_name == oPB.gns_name())
		{
			MYLOG_INFO(g_logger, "receive info from %s, %s, text %s", 
				oPB.gns_name().c_str(), oPB.ip_port().c_str(), oPB.log_text().c_str());			
			if(oCfg.logs.size() >= (size_t)g_conf.maxPerserveLogs)
			{
				//remove logs which has already been got (before log_idx)
				oCfg.logs.erase(oCfg.logs.begin(), oCfg.logs.begin() + oCfg.log_idx);
				oCfg.log_idx = 0;
				MYLOG_INFO(g_logger, "after erase, size is %lu", oCfg.logs.size());
				if(oCfg.logs.size() >= (size_t)g_conf.maxPerserveLogs)
				{
					MYLOG_WARN(g_logger, "overload, will not receive any logs");
					break;
				}
			}
					
			//add gns_name ip_port to text
			std::string text = oPB.gns_name() + " " + oPB.ip_port() + "  " + oPB.log_text();
			oCfg.logs.push_back(text);			
		}
	}
}

void AsyncCommThread::GetLog(std::string strContent, GM_CONN_INFO *pConn)
{
	::pb::GetLogReq oGetLogPB;
	if (!oGetLogPB.ParseFromString(strContent))
	{
		MYLOG_ERROR(g_logger, "ParseFromString failed!");
		return;
	}

	std::string ret_msg;
	int ret = 0;
	std::string log_text;
	::pb::GetLogResp oPB;

	do
	{
		auto itSession = g_map_session_info.find(oGetLogPB.session_id());
		if(itSession == g_map_session_info.end())
		{
			MYLOG_WARN(g_logger, "find no session %s", oGetLogPB.session_id().c_str());
			ret = ERROR_SESSION_NOT_FOUND;
			ret_msg = "session not found";
			break;
		}
		SESSION_INFO &oInfo = itSession->second;
		oInfo.active_time = time(0);
		GetLogByGnsEqually(oInfo, oPB);
	}while(0);
	
	//make res pb
	std::string strPbToSend;
	oPB.set_fd(oGetLogPB.fd());
	oPB.set_req_begin_time(oGetLogPB.req_begin_time());
	oPB.set_ret(ret);
	oPB.set_ret_msg(ret_msg);
	oPB.SerializeToString(&strPbToSend);

	SendMsg(strPbToSend, pConn->fd, GET_LOG_RESP);
}

void AsyncCommThread::SendCfgInfo2Clients(std::string session_id, std::string match, int match_type)
{
	SESSION_INFO oSessionInfo = g_map_session_info[session_id];
	
	for(auto it = oSessionInfo.cfgs.begin(); it != oSessionInfo.cfgs.end(); it++)
	{
		CFG oCfg = *it;
		
		//make pb
		std::string strPbToSend;
		::pb::CfgInfoReq oPB;
		oPB.set_session_id(session_id);
		oPB.set_p1(oCfg.p1);
		oPB.set_p2(oCfg.p2);
		oPB.set_p3(oCfg.p3);
		oPB.SerializeToString(&strPbToSend);

		if(CFG_IP_PORT == match_type)
		{
			if(oCfg.ip_port != match)
			{
				continue;
			}
			//only send cfg to machine with corresponding ip_port
			int fd = GetFdByIPPort(oCfg.ip_port);
            if(fd > 0)
            {
			    SendMsg(strPbToSend, fd, CFG_INFO_REQ);
            }
		}
		else if(CFG_GNS == match_type)
		{
			if(oCfg.gns_name != match)
			{	
				continue;
			}
			//find fd by gns_name
			std::set<int> set_fd = GetFdByGns(oCfg.gns_name);
			
			//send to client
			for(auto itFd = set_fd.begin(); itFd != set_fd.end(); itFd++)
			{
	            MYLOG_DEBUG(g_logger, "ready to send to %s, client fd:%d", oCfg.gns_name.c_str(), *itFd);
				SendMsg(strPbToSend, *itFd, CFG_INFO_REQ);			
			}
		}
		else
		{
			MYLOG_WARN(g_logger, "type %d not supported", match_type);
		}		
	}

}

void AsyncCommThread::SendCfg2SingleClient(CFG oCfg, std::string sesison_id, int fd)
{
	//make pb
	std::string strPbToSend;
	::pb::CfgInfoReq oPB;
	oPB.set_session_id(sesison_id);
	oPB.set_p1(oCfg.p1);
	oPB.set_p2(oCfg.p2);
	oPB.set_p3(oCfg.p3);
	oPB.SerializeToString(&strPbToSend);		
	SendMsg(strPbToSend, fd, CFG_INFO_REQ);
}

int AsyncCommThread::SendMsg(std::string msg, int fd, int msg_id)
{
	MSG_HEAD *pHead;
	size_t buf_len;
	size_t sent_len;

	GM_CONN_INFO* pConn = s_ConnMgr.m_mapFdConn[fd];
	if(NULL == pConn)
	{
		MYLOG_ERROR(g_logger, "connection not exist, fd:%d", fd);
		return ERROR_CONN_NOT_EXIST;
	}
	
	pHead = (MSG_HEAD *)pConn->read_buf;
	pHead->id = msg_id;
	pHead->len = msg.size();
	memcpy(pConn->read_buf + sizeof(MSG_HEAD), msg.data(), pHead->len);
	buf_len = pHead->len + sizeof(MSG_HEAD);
	pHead->len = htonl(pHead->len);
    pHead->id = htonl(pHead->id);
	
	sent_len = pConn->WriteData(fd, pConn->read_buf, buf_len, 100);
	if(sent_len == buf_len)
	{
		MYLOG_DEBUG(g_logger, "send to %d success", fd);
	}
	else
	{
		MYLOG_WARN(g_logger, "send to %d failed, erron:%d, req len:%lu, real len:%lu", 
			fd, errno, buf_len, sent_len);
		return ERROR_SEND_FAILED;
	}

	return ERROR_OK;
}


