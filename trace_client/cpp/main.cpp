#include <string>
#include <iostream>
#include <thread>
#include "tracelog.h"
#include "getlocalip.h"

using namespace std;

CGMApp *g_pTracer = NULL;
log4cxx::LoggerPtr g_logger(log4cxx::Logger::getRootLogger());

void test_trace()
{
    TRACE(g_pTracer, "%s trace unconditionally to all sessions", "TRACE");

    uint64_t p2_check = 8888;
    TRACE_IF(g_pTracer, (const char*)&p2_check, "%s trace if p2 equals p2_check to all sessions", "TRACE_IF");

    //TRACE_SESSION usually used in tracelog.cpp
    
    bool b = true;
    TRACE_COND(g_pTracer, b, "%s trace if b is true to all sessions", "TRACE_COND");
    int i = 1;
    TRACE_COND(g_pTracer, b, "%s trace if i is not 0 to all sessions", "TRACE_COND");
}

void main_thread(string msg)
{
    while(1)
    {
        test_trace();
        sleep(10);
    }
}

int main()
{
    try
    {
        log4cxx::PropertyConfigurator::configure("../etc/log4cxx.cfg");
        setlocale(LC_ALL, "");
    }
    catch (log4cxx::helpers::Exception&)
    {
        return false;
    }

    g_pTracer = new CGMApp();
    g_pTracer->g_pLogger = g_logger;
    g_pTracer->SetOpenRunLog();
    
    initTraceApp(g_pTracer);    //start thread
    usleep(10000); //make enough time for thread to init

    std::string selfIp;
    GetOneSelfPublicIpAddress(selfIp);
 
    reportSelfInfo(g_pTracer, "MY_SERVICE", selfIp, 8888, "120.25.166.70", 20009);

    thread t(main_thread, "Hello");

    t.join();
}
