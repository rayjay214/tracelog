#ifndef _HELPER_H__
#define _HELPER_H__

#include "trace_typedef.h"
#include <set>
#include <string>
#include <map>
#include "trace.pb.h"
#include <openssl/md5.h>

#define MAX_GET_LOGS (50)
#define MAX_PERSERVE_LOGS (50)

typedef struct tagConfig
{
	std::string password;
	int maxGetLogs;
	int maxPerserveLogs;
	int sessionExpireSeconds;
	int listenPort;
}Config;

int MakeMD5(const std::string& rawData, std::string& md5Data);
std::set<int> GetFdByGns(std::string gns_name);
int GetFdByIPPort(std::string ip_port);
void GetLogByGnsEqually(SESSION_INFO &oInfo, ::pb::GetLogResp &oPB);


extern std::map<int, CLIENT_INFO> g_map_fd_cli_info;
extern std::map<std::string, SESSION_INFO> g_map_session_info;
extern Config g_conf;

#endif /*_HELPER_H__*/

