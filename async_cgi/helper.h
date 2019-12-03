#ifndef _HELPER_H__
#define _HELPER_H__
#include "type.h"
#include "trace.pb.h"

typedef struct tagConfig
{
	std::string traceServerIp;
	int traceServerPort;
	int listenPort;
}Config;


#define HTML_HEAD "Content-Type: text/html; charset=UTF-8\r\n\r\n"
#define HTML_HEAD_TXT "Content-Type: application/text; charset=UTF-8\r\n\r\n"
#define HTML_HEAD_JS "Content-Type: text/javascript; charset=UTF-8\r\n\r\n"
#define HTML_HEAD_EXCEL "Content-Disposition:attachment; filename=test.xls\r\n\r\n"  \
				        "Content-Type:application/vnd.ms-excel; charset=UTF-8\r\n\r\n"

#define RES_TXT (1)
#define RES_JS (2)
#define RES_EXCEL (3)

std::string GetHttpResponse(std::string res, int resType);
std::string GenLoginRespJson(::pb::LoginResp oPB);
std::string GenSetConfigRespJson(::pb::CfgInfoResp oPB);
std::string GenGetLogRespJson(::pb::GetLogResp oPB);

extern Config g_conf;

#endif /*_HELPER_H__*/

