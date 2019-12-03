#include "tracelog.h"
#include <inttypes.h>

bool CTraceLog::check(const CfgInfo& cfg, const char *checkif)
{
    uint64_t id;
    switch (cfg.p1)
    {
    case TRACE_LOG_SET_ID:
        id = *(uint64_t*)checkif;
        if (id == (uint64_t)cfg.p2)
            return true;
        break;
    default:
        break;
    }

    return false;
}

void CTraceLog::setConfig(std::string strContent)
{
	::pb::CfgInfoReq oCfgPB;
	if (!oCfgPB.ParseFromString(strContent))
	{
	    MYLOG_ERROR(CGMApp::g_pLogger, "ParseFromString failed!");
		return;
	}
    int op_type = oCfgPB.p1();
    switch (op_type)
    {
    case TRACE_LOG_SET_ID:
        SaveConfig(oCfgPB);
        TRACE_SESSION(g_pTracer, oCfgPB.session_id(), "set trace %ld success", oCfgPB.p2());
        break;
    default:
        break;
    }
}

//non-class member functions
void* runThread(void *thr)
{
    CGMApp* pApp = (CGMApp*)thr;
    assert(pApp);
    pApp->Run("",0);
    return (void*)0;  
}

//netframe client init
void initTraceApp(CGMApp* pApp)
{       
	CTraceLog *pTraceLogThr = new CTraceLog();
	pApp->AddThread(pTraceLogThr, "TraceLog", 0, 0);
	
    // run as client
    pthread_t threadID;
    if(CreateThread(runThread, pApp, &threadID, 0, 0) != 0)
    {
        MYLOG_ERROR(CGMApp::g_pLogger, "create thread failed, errno:%d.", errno);
    }
}

void reportSelfInfo(CGMApp* pApp, std::string serviceName, std::string selfIp, int selfPort, std::string traceServerIp, int traceServerPort)
{
	TraceClientThread *pTraceClientThr = (TraceClientThread*)pApp->m_Threads[0];
	pTraceClientThr->m_gns_name.push_back(serviceName);
	pTraceClientThr->m_ip_port.push_back(selfIp + ":" + ToString(selfPort));
	pTraceClientThr->m_trace_server_ip = traceServerIp;
    pTraceClientThr->m_trace_server_port = traceServerPort;
	pTraceClientThr->ReportSelfInfo();
}

