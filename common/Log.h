#ifndef CLOG_H
#define CLOG_H

#include <string>
#include <stdarg.h>
#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/helpers/exception.h"
//#include "log4cxx/fileappender.h"
#include <log4cxx/rollingfileappender.h>
#include "log4cxx/patternlayout.h"

#define MYLOG_DEBUG(logger,fmt,...) CLog::LogDebug(logger,__FILE__,__FUNCTION__,__LINE__,fmt,##__VA_ARGS__)
#define MYLOG_INFO(logger,fmt,...) CLog::LogInfo(logger,__FILE__,__FUNCTION__,__LINE__,fmt,##__VA_ARGS__)
#define MYLOG_WARN(logger,fmt,...) CLog::LogWarn(logger,__FILE__,__FUNCTION__,__LINE__,fmt,##__VA_ARGS__)
#define MYLOG_ERROR(logger,fmt,...) CLog::LogError(logger,__FILE__,__FUNCTION__,__LINE__,fmt,##__VA_ARGS__)
#define MYLOG_FATAL(logger,fmt,...) CLog::LogFatal(logger,__FILE__,__FUNCTION__,__LINE__,fmt,##__VA_ARGS__)

#define MYLOG_SET_OPTION(logger,option,value) CLog::setOption(logger,option,value)

#define MYLOG_INIT(cfg,mod,lev) CLog::Init(cfg,mod,lev)

class CLog
{
public:
    enum ELOG_LEVEL
    {
        EDEBUG  = 0,
        EINFO   = 1,
        EWARN   = 2,
        EERROR  = 3,
        EFATAL  = 4
    };

public:
    CLog();
    virtual ~CLog();
    static log4cxx::LoggerPtr Init(std::string &cfg,std::string &mod,ELOG_LEVEL lev = EINFO);
    static log4cxx::LoggerPtr Init(const char *cfg,const char *mod,ELOG_LEVEL lev = EINFO);

    static void setOption(log4cxx::LoggerPtr &logger,const char *option,const char *value);
    static void setOption(log4cxx::LoggerPtr &logger,std::string &option,std::string &value);

    static void LogDebug(log4cxx::LoggerPtr &logger,const char* pFile,const char *pFunc,int line,const char* pInfo,...) __attribute__((format(printf,5,6)));
    static void LogInfo(log4cxx::LoggerPtr &logger,const char* pFile,const char *pFunc,int line,const char* pInfo,...) __attribute__((format(printf,5,6)));
    static void LogWarn(log4cxx::LoggerPtr &logger,const char* pFile,const char *pFunc,int line,const char* pInfo,...) __attribute__((format(printf,5,6)));
    static void LogError(log4cxx::LoggerPtr &logger,const char* pFile,const char *pFunc,int line,const char* pInfo,...) __attribute__((format(printf,5,6)));
    static void LogFatal(log4cxx::LoggerPtr &logger,const char* pFile,const char *pFunc,int line,const char* pInfo,...) __attribute__((format(printf,5,6)));

private:
    static int setFileOption(log4cxx::LoggerPtr &logger,int type,std::string &value,int num = 0);
    static void setLevel(log4cxx::LoggerPtr &logger,ELOG_LEVEL lev);
    static void getProcessName(std::string &str);
};

#endif // CLOG_H
