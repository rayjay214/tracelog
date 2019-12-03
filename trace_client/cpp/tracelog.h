#include "TraceClientThread.h"

enum TRACE_LOG_ENUM
{
    TRACE_LOG_SET_ID = 1
};

class CTraceLog : public TraceClientThread
{
public:
    CTraceLog()
    {
    }

    ~CTraceLog()
    {
    }

    virtual bool check(const CfgInfo& cfg, const char *checkif);

protected:
    virtual void setConfig(std::string strContent);
    
};

void* runThread(void *thr);
void initTraceApp(CGMApp* pApp);
void reportSelfInfo(CGMApp* pApp, std::string serviceName, std::string selfIp, int selfPort, std::string traceServerIp, int traceServerPort);

extern CGMApp* g_pTracer;

