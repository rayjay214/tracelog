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
#include "trace.pb.h"
#include "cgicc_lib/Cgicc.h"
#include <boost/algorithm/string.hpp>
#include "helper.h"

#define TIMER_ID_CHECK_FD 1
#define TIMER_CHECK_FD_INTERVAL 30000

#define TIMER_ID_CHECK_FCGI_OVERTIME 2
#define TIMER_CHECK_FCGI_OVERTIME_INTERVAL 1000

#define TIMER_ID_HEARTBEAT 3
#define TIMER_ID_HEARTBEAT_INTERVAL 120000


using namespace muduo::net;

AsyncCommThread::AsyncCommThread()
{
    m_pCon = NULL;
	//CreateConnectionToServer();
}

AsyncCommThread::~AsyncCommThread()
{
}

// thread already started.  for initializing something
int AsyncCommThread::OnThreadStarted()
{
    SetTimer(TIMER_ID_CHECK_FD, 0, 0, TIMER_CHECK_FD_INTERVAL, TIMER_TYPE_PERMENENT);
	SetTimer(TIMER_ID_CHECK_FCGI_OVERTIME, 0, 0, TIMER_CHECK_FCGI_OVERTIME_INTERVAL, TIMER_TYPE_PERMENENT);
	SetTimer(TIMER_ID_HEARTBEAT, 0, 0, TIMER_ID_HEARTBEAT_INTERVAL, TIMER_TYPE_PERMENENT);
    // return none-zero will cause initialization failed.
    CreateConnectionToServer();
    return 0;
}

// time out function.
void AsyncCommThread::OnTimer(UINT32 timerid, UINT32 par1, void *par2)
{
    switch(timerid)
    {
    /*gm框架中没有包含 fd检测，仅提供了CheckFDExpireTime函数*/
    case TIMER_ID_CHECK_FD:
        CheckFDExpireTime(120);
        break;
	case TIMER_ID_CHECK_FCGI_OVERTIME:
		CheckFCGIOvertimeReq(10);
		break;
	case TIMER_ID_HEARTBEAT:
		SendHeartBeatMsg();
		break;
	
    default:
        break;
    }
}

void AsyncCommThread::OnNetMsg(int fd, UINT32 revents, void *handler)
{
    MYLOG_INFO(g_logger, "on net msg, fd:%d, revnets:%d", fd, revents);
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

    /*注意这一行与基类代码的不同  sizeof(MSG_HEAD)*/
    int ret = OnMsgOneConDeal(fd, revents, pCon, kRecordHeader);
    if(ret < 0)
    {
        //pCon就是m_pCon
        assert(pCon == m_pCon);
        m_pCon = NULL;
    }

}


// get something from local
void AsyncCommThread::OnLocalMsg(char *pMsg)
{
}


void AsyncCommThread::CreateConnectionToServer()
{
	if(!m_pCon)
	{
		MYLOG_WARN(g_logger, "connection not exist, reconnect");
		m_pCon = NewConnectionBlock();
		m_pCon->AddRef();
		m_pCon->InitializeBuf(102400, 102400);
		CreateProxyConnection(g_conf.traceServerIp, g_conf.traceServerPort);
	}
}

void AsyncCommThread::SendHeartBeatMsg()
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

void AsyncCommThread::CheckFCGIOvertimeReq(int timeoutInSeconds)
{
	time_t tNow = time(0);
	CGMSafePLock Locker(&s_ConnMgr.m_MtxFD);
	for(auto it = s_ConnMgr.m_mapFdConn.begin(); it != s_ConnMgr.m_mapFdConn.end(); it++)
	{
		SelfConn* pSelfConn = (SelfConn*)it->second;
		if(pSelfConn == NULL)
			return;
		MYLOG_DEBUG(g_logger, "fd %d, got %d, begin_time:%lu", it->first, pSelfConn->m_pCodec->gotRequest_, pSelfConn->m_pCodec->reqBeginTime_);
		if(pSelfConn->m_pCodec->gotRequest_ && ((tNow - pSelfConn->m_pCodec->reqBeginTime_) > timeoutInSeconds) )
		{
			//send timeout resp to nginx, if receive resp from server afterwards, discard it
			std::string result = "backend server timeout";
			result = GetHttpResponse(result, RES_TXT);
			muduo::net::Buffer response;
		    respond(result, &response);
			size_t sent_len = pSelfConn->WriteData(pSelfConn->fd, (unsigned char*)response.peek(), response.readableBytes(), 100);
			if(sent_len != response.readableBytes())
			{
				MYLOG_WARN(g_logger, "write failed, req len %lu, real len %lu", response.readableBytes(), sent_len);
			}
			ResetConnStatus(pSelfConn);
		}
	}
}

int AsyncCommThread::SetSrvSocketOpts(int socket)
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

int AsyncCommThread::CreateProxyConnection(std::string serverIp, int serverPort)
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
        if(CODE_OK !=  SetupConnection(serverIp, serverPort, m_pCon))
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

	//这一行非常重要，服务端断连之后，客户端销毁connection的时候会把其置为1
	//再次重用此connection跟服务端建链之后，如果bWaitClose还是1，就会导致客户端无端的close fd
	m_pCon->bWaitClose = 0;
	usleep(1000); //wait for connection to be ready
	
    SetSrvSocketOpts(m_pCon->fd);
    return CODE_OK;
}

bool AsyncCommThread::ParseNetMsg(int fd, GM_CONN_INFO *pCon, int &readlen) 
{
    MYLOG_DEBUG(g_logger, "enter parse");
    readlen = 0;
    if(kRecordHeader > (size_t)pCon->read_idx)
    {
        MYLOG_WARN(g_logger, "incomplete, read_idx %d", pCon->read_idx);
        return true;
    }
    
    RecordHeader *pHead = (RecordHeader *)pCon->read_buf;

	if(pHead->version != 1)  //表示消息不是从nginx来的，用标准消息头解析
	{
		return HandleTraceServerMsg(pCon, readlen);
	}
	else
	{
		return HandleNginxMsg(pHead, pCon, readlen);	
	}
}

bool AsyncCommThread::HandleTraceServerMsg(GM_CONN_INFO *pCon, int &readlen)
{
	MSG_HEAD *pNormalHead = (MSG_HEAD *)pCon->read_buf;
	pNormalHead->id = be32toh(pNormalHead->id);
	pNormalHead->len = be32toh(pNormalHead->len);
	if(pNormalHead->len + sizeof(MSG_HEAD) > (size_t)pCon->read_idx)
	{
        MYLOG_WARN(g_logger, "incomplete body len %d, read_idx %d", pNormalHead->len, pCon->read_idx);
		return true;
	}
	readlen = pNormalHead->len + sizeof(MSG_HEAD);

	std::string strReaded((char*)(pCon->read_buf + sizeof(MSG_HEAD)), pNormalHead->len);
	
	HandleMsg(pNormalHead, strReaded);
	return true;
}

bool AsyncCommThread::HandleNginxMsg(RecordHeader *pHead, GM_CONN_INFO *pCon, int &readlen)
{
	pHead->id = be16toh(pHead->id);
    pHead->length = be16toh(pHead->length);
    size_t total = kRecordHeader + pHead->length + pHead->padding;

    if(total > (size_t)pCon->read_idx)
    {
        return true;
    }

    readlen = total;

	SelfConn *pSelfCon = reinterpret_cast<SelfConn *>(pCon);
	if(pSelfCon->m_pCodec->gotRequest_)
	{
		onRequest(pSelfCon);
		pSelfCon->m_pCodec->stdin_.retrieveAll();
      	pSelfCon->m_pCodec->paramsStream_.retrieveAll();
      	pSelfCon->m_pCodec->params_.clear();
      	pSelfCon->m_pCodec->gotRequest_ = false;
	}

	switch (pHead->type)
    {
    case kFcgiBeginRequest:
        if(!onBeginRequest(pHead, (const char*)pCon->read_buf + kRecordHeader, pHead->length, pCon))
       	{
       		MYLOG_ERROR(g_logger, "something wrong with the protocal, close the connection");
			pCon->bWaitClose = 1; //something wrong with the protocal, close the connection
        }
        break;
    case kFcgiParams:
        if(!onParams((const char*)pCon->read_buf + kRecordHeader, pHead->length, pCon))
        {
        	MYLOG_ERROR(g_logger, "something wrong with the protocal, close the connection");
			pCon->bWaitClose = 1; //something wrong with the protocal, close the connection
        }
        break;
    case kFcgiStdin:
        onStdin((const char*)pCon->read_buf + kRecordHeader, pHead->length, pCon);
        break;
    case kFcgiData:
        break;
    case kFcgiGetValues:
        break;
    default:
    	MYLOG_ERROR(g_logger, "something wrong with the protocal, close the connection");
		pCon->bWaitClose = 1; //something wrong with the protocal, close the connection
        break;
    }

	return true;
}

void AsyncCommThread::HandleMsg(MSG_HEAD *pHead, std::string strContent)
{
	switch (pHead->id)
	{
		case LOGIN_RESP:
		{
	        LoginResp(strContent);		
			break;
		}
		case SET_CONF_RESP:
		{
			SetConfigResp(strContent);
			break;
		}
		case GET_LOG_RESP:
		{
			GetLogResp(strContent);
			break;
		}
		case HEARTBEAT_MSG:
		{
			//do nothing for the moment
			MYLOG_DEBUG(g_logger, "receive heartbeat msg from server");
			break;
		}
		default:
			break;	
	}
}

void AsyncCommThread::ResetConnStatus(SelfConn* pConn)
{
	pConn->m_pCodec->gotRequest_ = false;
	pConn->m_pCodec->reqBeginTime_ = 0;
	pConn->m_pCodec->stdin_.retrieveAll();
    pConn->m_pCodec->paramsStream_.retrieveAll();
    pConn->m_pCodec->params_.clear();
}

bool AsyncCommThread::CheckConn(int fd, time_t check_time)
{
	SelfConn* pConn = reinterpret_cast<SelfConn *>(s_ConnMgr.m_mapFdConn[fd]);  //找到与nginx对应的connection
	if(pConn == NULL)
	{
		MYLOG_WARN(g_logger, "connection is null, fd:%d", fd);
		return false;
	}
	if(check_time != pConn->m_pCodec->reqBeginTime_) //表示reqBeginTime_已重置，可能超时，可能链接已被复用
	{
		MYLOG_WARN(g_logger, "original conn has been destoryed, fd:%d, real req_time:%lu, current time:%lu", 
			fd, check_time, pConn->m_pCodec->reqBeginTime_);	
		return false;
	}

	return true;
}

void AsyncCommThread::LoginResp(std::string strContent)
{
	::pb::LoginResp oPB;
	if (!oPB.ParseFromString(strContent))
	{
		MYLOG_ERROR(g_logger, "ParseFromString failed!");
		return;
	}

	if(!CheckConn(oPB.fd(), oPB.req_begin_time()))
	{
		MYLOG_WARN(g_logger, "connection check failed!");
		return;
	}

	SelfConn* pConn = reinterpret_cast<SelfConn *>(s_ConnMgr.m_mapFdConn[oPB.fd()]);
	
	std::string result = GenLoginRespJson(oPB);
	result = GetHttpResponse(result, RES_JS);
	muduo::net::Buffer response;
    respond(result, &response);
	size_t sent_len = pConn->WriteData(pConn->fd, (unsigned char*)response.peek(), response.readableBytes(), 100);
	if(sent_len != response.readableBytes())
	{
		MYLOG_WARN(g_logger, "write failed, req len %lu, real len %lu", response.readableBytes(), sent_len);
	}
	ResetConnStatus(pConn);
	
}

void AsyncCommThread::SetConfigResp(std::string strContent)
{
	::pb::CfgInfoResp oPB;
	if (!oPB.ParseFromString(strContent))
	{
		MYLOG_ERROR(g_logger, "ParseFromString failed!");
		return;
	}

	if(!CheckConn(oPB.fd(), oPB.req_begin_time()))
	{
		MYLOG_WARN(g_logger, "connection check failed!");
		return;
	}

	SelfConn* pConn = reinterpret_cast<SelfConn *>(s_ConnMgr.m_mapFdConn[oPB.fd()]);  //找到与nginx对应的connection
	
	std::string result = GenSetConfigRespJson(oPB);
	result = GetHttpResponse(result, RES_JS);
	muduo::net::Buffer response;
    respond(result, &response);
	size_t sent_len = pConn->WriteData(pConn->fd, (unsigned char*)response.peek(), response.readableBytes(), 100);
	if(sent_len != response.readableBytes())
	{
		MYLOG_WARN(g_logger, "write failed, req len %lu, real len %lu", response.readableBytes(), sent_len);
	}
	ResetConnStatus(pConn);
}

void AsyncCommThread::GetLogResp(std::string strContent)
{
	::pb::GetLogResp oPB;
	if (!oPB.ParseFromString(strContent))
	{
		MYLOG_ERROR(g_logger, "ParseFromString failed!");
		return;
	}

	if(!CheckConn(oPB.fd(), oPB.req_begin_time()))
	{
		MYLOG_WARN(g_logger, "connection check failed!");
		return;
	}

	SelfConn* pConn = reinterpret_cast<SelfConn *>(s_ConnMgr.m_mapFdConn[oPB.fd()]);  //找到与nginx对应的connection
	
	std::string result = GenGetLogRespJson(oPB);
	result = GetHttpResponse(result, RES_JS);
	muduo::net::Buffer response;
    respond(result, &response);
	size_t sent_len = pConn->WriteData(pConn->fd, (unsigned char*)response.peek(), response.readableBytes(), 100);
	if(sent_len != response.readableBytes())
	{
		MYLOG_WARN(g_logger, "write failed, req len %lu, real len %lu", response.readableBytes(), sent_len);
	}
	ResetConnStatus(pConn);
}

void AsyncCommThread::onRequest(SelfConn *pSelfCon)
{
	SSMap params = pSelfCon->m_pCodec->params_;

	std::string queryString = params["QUERY_STRING"];
    std::string queryType   = params["CONTENT_TYPE"];
    std::string queryCookie = params["HTTP_COOKIE"];
    std::string postData;
    if (pSelfCon->m_pCodec->stdin_.readableBytes() > 0)
    {
        postData = pSelfCon->m_pCodec->stdin_.retrieveAllAsString();
    }

    SSMap qmap;
    // Parse Url String
    // Parse Post Data With Standard content type = application/x-www-form-urlencoded 
    cgicc::Cgicc cgi(queryString, queryCookie, postData, queryType);
    cgicc::const_form_iterator iterE;
    for(iterE = cgi.getElements().begin(); iterE != cgi.getElements().end(); ++iterE) 
    {   
        std::string key = boost::to_lower_copy(iterE->getName());
        qmap.insert(SSMap::value_type(key, iterE->getValue()));
        MYLOG_INFO(g_logger, "key:%s, value:%s", key.c_str(), iterE->getValue().c_str());
        //LOG_DEBUG << "Query Element:" << key << " Value:" << iterE->getValue();
    }

    // Parse File Uplod, File upload type = multipart/form-data 
    cgicc::const_file_iterator  iterF;
    for(iterF = cgi.getFiles().begin(); iterF != cgi.getFiles().end(); ++iterF)
    {
        qmap.insert(SSMap::value_type(iterF->getName(), iterF->getData()));
    }

    SSMap header;
    for (FastCgiCodec::ParamMap::const_iterator it = params.begin(); it != params.end(); ++it)
    {
        header[it->first] = it->second;
        //LOG_DEBUG << "Query Headers " << it->first << " = " << it->second;
    }

    // Parse Cookie List
    cgicc::const_cookie_iterator iterV;
    for(iterV = cgi.getCookieList().begin(); iterV != cgi.getCookieList().end(); ++iterV)
    {
        header[iterV->getName()] = iterV->getValue();
        //LOG_DEBUG << "Cookie Param: Name["<< iterV->getName() << "] Value[" << iterV->getValue() << "].";
    }

	//todo, async way
	std::string method = qmap["method"];
	if(method == "login")
	{
		LoginReq(pSelfCon, qmap);
	}
	else if(method == "setConfig")
	{
	    SetConfig(pSelfCon, qmap);	
	}
	else if(method == "getLog")
	{
		GetLog(pSelfCon, qmap);
	}
	
}

void AsyncCommThread::LoginReq(SelfConn *pSelfCon, SSMap qmap)
{
	//construct msg to send
	std::string strLoginReq;
	::pb::LoginReq oPB;
	oPB.set_password(qmap["password"]);
	oPB.set_fd(pSelfCon->fd);  //响应回来后，用fd和time找cgi和nginx间对应的connection
	oPB.set_req_begin_time(pSelfCon->m_pCodec->reqBeginTime_);
	oPB.SerializeToString(&strLoginReq);
	
	SendMsg(strLoginReq, LOGIN_REQ);
}

void AsyncCommThread::SetConfig(SelfConn *pSelfCon, SSMap qmap)
{
	//construct msg to send
	std::string strPbToSend;
	::pb::CfgInfoReq oPB;
	oPB.set_session_id(qmap["session_id"]);
	oPB.set_p1(atol(qmap["p1"].c_str()));
	oPB.set_p2(atol(qmap["p2"].c_str()));
	oPB.set_p3(qmap["p3"]);
	oPB.set_gns_name(qmap["gns_name"]);
	oPB.set_ip_port(qmap["ip_port"]);
	oPB.set_fd(pSelfCon->fd);  //响应回来后，用fd和time找cgi和nginx间对应的connection
	oPB.set_req_begin_time(pSelfCon->m_pCodec->reqBeginTime_);
	oPB.SerializeToString(&strPbToSend);
	
	SendMsg(strPbToSend, SET_CONF_REQ);
}

void AsyncCommThread::GetLog(SelfConn *pSelfCon, SSMap qmap)
{
	//construct msg to send
	std::string strPbToSend;
	::pb::GetLogReq oPB;
	oPB.set_session_id(qmap["session_id"]);
	oPB.set_gns_name(qmap["gns_name"]);
    oPB.set_seq(atol(qmap["seq"].c_str()));
	oPB.set_fd(pSelfCon->fd);  //响应回来后，用fd和time找cgi和nginx间对应的connection
	oPB.set_req_begin_time(pSelfCon->m_pCodec->reqBeginTime_);
	oPB.SerializeToString(&strPbToSend);
	
	SendMsg(strPbToSend, GET_LOG_REQ);
}


int AsyncCommThread::SendMsg(std::string msg, int msg_id)
{
	if(!m_pCon)
	{
		MYLOG_WARN(g_logger, "send to server failed, no connection.");
		return -1;
	}
	
	MSG_HEAD *pHead;
	size_t buf_len;
	size_t sent_len;

	pHead = (MSG_HEAD *)m_pCon->read_buf;
	pHead->id = msg_id;
	pHead->len = msg.size();
	memcpy(m_pCon->read_buf + sizeof(MSG_HEAD), msg.data(), pHead->len);
	buf_len = pHead->len + sizeof(MSG_HEAD);
	pHead->len = htobe32(pHead->len);
    pHead->id = htobe32(pHead->id);
	
	sent_len = m_pCon->WriteData(m_pCon->fd, m_pCon->read_buf, buf_len, 100);
	if(sent_len == buf_len)
	{
		MYLOG_DEBUG(g_logger, "send to %d success", m_pCon->fd);
	}
	else
	{
		MYLOG_WARN(g_logger, "send to %d failed, erron:%d, req len:%lu, real len:%lu", 
			m_pCon->fd, errno, buf_len, sent_len);
        ReleaseConnection(m_pCon);
        m_pCon = NULL;
		return -1;
	}

	return 0;
}

void AsyncCommThread::respond(std::string &out, Buffer* response)
{
    if(out.empty())
    {
        std::string content = "Empty Result!";
        std::ostringstream resp;
        resp << "Content-Type: text/html; charset=UTF-8\r\n";
        resp << "Content-Length: " << content.length() << "\r\n\r\n";
        resp << content;
        out = resp.str();
    }

    if (out.length() <= FCGI_MAX_LENGTH)
    {
        RecordHeader header =
        {
            1,
            kFcgiStdout,
            htobe16(1),
            htobe16(static_cast<uint16_t>(out.length())),
            static_cast<uint8_t>(-out.length() & 7),
            0,
        };
        response->append(&header, kRecordHeader);
        response->append(out);
        response->append("\0\0\0\0\0\0\0\0", header.padding);
    }
    else
    {
        size_t numSubstrings = out.length() / FCGI_MAX_LENGTH;
        for (size_t i = 0; i < numSubstrings; i++)
        {
            std::string outSubString = out.substr(i * FCGI_MAX_LENGTH, FCGI_MAX_LENGTH);
            RecordHeader header =
            {
                1,
                kFcgiStdout,
                htobe16(1),
                htobe16(static_cast<uint16_t>(FCGI_MAX_LENGTH)),
                static_cast<uint8_t>(-FCGI_MAX_LENGTH & 7),
                0,
            };
            response->append(&header, kRecordHeader);
            response->append(outSubString);
            response->append("\0\0\0\0\0\0\0\0", header.padding);
        }

        if (out.length() % FCGI_MAX_LENGTH != 0)
        {
            std::string outSubString = out.substr(FCGI_MAX_LENGTH * numSubstrings);
            RecordHeader header =
            {
                1,
                kFcgiStdout,
                htobe16(1),
                htobe16(static_cast<uint16_t>(outSubString.length())),
                static_cast<uint8_t>(-outSubString.length() & 7),
                0,
            };
            response->append(&header, kRecordHeader);
            response->append(outSubString);
            response->append("\0\0\0\0\0\0\0\0", header.padding);
        }
    }

    endStdout(response);
    endRequest(response);
}

void AsyncCommThread::endStdout(Buffer* buf)
{
    RecordHeader header =
    {
        1,
        kFcgiStdout,
        htobe16(1),
        0,
        0,
        0,
    };
    buf->append(&header, kRecordHeader);
}

void AsyncCommThread::endRequest(Buffer* buf)
{
    RecordHeader header =
    {
        1,
        kFcgiEndRequest,
        htobe16(1),
        htobe16(kRecordHeader),
        0,
        0,
    };
    buf->append(&header, kRecordHeader);
    buf->appendInt32(0);
    buf->appendInt32(0);
}


bool AsyncCommThread::onBeginRequest(RecordHeader* pHead, const char* pContent, 
	size_t length, GM_CONN_INFO *pCon)
{
    assert(pHead->type == kFcgiBeginRequest);
	
    SelfConn *pSelfCon = reinterpret_cast<SelfConn *>(pCon);

    if (pHead->length >= kRecordHeader)
    {
        uint16_t role = be16toh(*(uint16_t *)pContent);
        uint8_t flags = *(uint8_t *)(pContent + sizeof(uint16_t));
        if (role == kFcgiResponder)
        {
            pSelfCon->m_pCodec->keepConn_ = flags == kFcgiKeepConn;
			pSelfCon->m_pCodec->reqBeginTime_ = time(0);
            return true;
        }
		pSelfCon->m_pCodec->reqBeginTime_ = time(0);
    }
    return false;
}

void AsyncCommThread::onStdin(const char* pContent, uint16_t length, GM_CONN_INFO *pCon)
{
	MYLOG_WARN(g_logger, "enter stdin");
	
	SelfConn *pSelfCon = reinterpret_cast<SelfConn *>(pCon);

	if (length > 0)
	{
		pSelfCon->m_pCodec->stdin_.append(pContent, length);
	}
	else
	{
		pSelfCon->m_pCodec->gotRequest_ = true;
		onRequest(pSelfCon);
	}
}

bool AsyncCommThread::onParams(const char* pContent, uint16_t length, GM_CONN_INFO *pCon)
{
	SelfConn *pSelfCon = reinterpret_cast<SelfConn *>(pCon);
	
    if (length > 0)
    {
        pSelfCon->m_pCodec->paramsStream_.append(pContent, length);
    }
    else if (!parseAllParams(pCon))
    {
        return false;
    }
    return true;
}

bool AsyncCommThread::parseAllParams(GM_CONN_INFO *pCon)
{
	SelfConn *pSelfCon = reinterpret_cast<SelfConn *>(pCon);
	
    while (pSelfCon->m_pCodec->paramsStream_.readableBytes() > 0)
    {
        uint32_t nameLen = readLen(pCon);
        if (nameLen == static_cast<uint32_t>(-1))
            return false;
        uint32_t valueLen = readLen(pCon);
        if (valueLen == static_cast<uint32_t>(-1))
            return false;
        if (pSelfCon->m_pCodec->paramsStream_.readableBytes() >= nameLen+valueLen)
        {
            string name = pSelfCon->m_pCodec->paramsStream_.retrieveAsString(nameLen);
			string value = pSelfCon->m_pCodec->paramsStream_.retrieveAsString(valueLen);
            pSelfCon->m_pCodec->params_[name] = value;
			MYLOG_WARN(g_logger, "name:%s, value:%s", name.c_str(), value.c_str());
        }
        else
        {
            return false;
        }
    }
    return true;
}

uint32_t AsyncCommThread::readLen(GM_CONN_INFO *pCon)
{
	SelfConn *pSelfCon = reinterpret_cast<SelfConn *>(pCon);

    if (pSelfCon->m_pCodec->paramsStream_.readableBytes() >= 1)
    {
        uint8_t byte = pSelfCon->m_pCodec->paramsStream_.peekInt8();
        if (byte & 0x80)
        {
            if (pSelfCon->m_pCodec->paramsStream_.readableBytes() >= sizeof(uint32_t))
            {
                return pSelfCon->m_pCodec->paramsStream_.readInt32() & 0x7fffffff;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            return pSelfCon->m_pCodec->paramsStream_.readInt8();
        }
    }
    else
    {
        return -1;
    }
}

