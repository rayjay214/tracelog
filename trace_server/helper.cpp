#include "helper.h"

int MakeMD5(const std::string& rawData, std::string& md5Data)
{
    uint32_t len = rawData.length();
    char tmp[3] = {0};
    char buf[33]= {0};
    uint8_t md[16] = {0};

    // malloc memory
    uint8_t* data = new uint8_t[len];
    memcpy(data, rawData.c_str(), len);
    if (NULL == MD5(data, rawData.length(), md))
    {
        // free memory
        delete[] data;
        md5Data = "set md5 fail";
        return -1;
    }

    // free memory
    delete[] data;
    for (int i = 0; i < 16; i++)
    {
        sprintf(tmp, "%2.2x", md[i]);
        strcat(buf,tmp);
    }

    md5Data = buf;
    return 0;
}


std::set<int> GetFdByGns(std::string gns_name)
{
	std::set<int> set_fd;
	for(auto it = g_map_fd_cli_info.begin(); it != g_map_fd_cli_info.end(); it++)
	{
		int fd = it->first;
		CLIENT_INFO oInfo = it->second;
		for(auto itGns = oInfo.gns_names.begin(); itGns != oInfo.gns_names.end(); itGns++)
		{
			if(*itGns == gns_name)
			{
				set_fd.insert(fd);
			}
		}
	}
    return set_fd;
}

int GetFdByIPPort(std::string ip_port)
{
	for(auto it = g_map_fd_cli_info.begin(); it != g_map_fd_cli_info.end(); it++)
	{
		int fd = it->first;
		CLIENT_INFO oInfo = it->second;
		for(auto itIpPort = oInfo.ip_ports.begin(); itIpPort != oInfo.ip_ports.end(); itIpPort++)
		{
			if(*itIpPort == ip_port)
			{
				return fd;
			}
		}
	}
    return -1;
}

void GetLogByGnsEqually(SESSION_INFO &oInfo, ::pb::GetLogResp &oPB)
{
	size_t cfg_cnt = oInfo.cfgs.size();
	size_t get_max_num = g_conf.maxGetLogs / cfg_cnt;
	
	for(auto it = oInfo.cfgs.begin(); it != oInfo.cfgs.end(); it++)
	{
		CFG &oCfg = *it;
		::pb::LogText* pLogText =  oPB.add_log_text();
		pLogText->set_ip_port(oCfg.ip_port);				
		pLogText->set_gns_name(oCfg.gns_name);
		size_t cnt = 0;
		for(auto itLog = oCfg.logs.begin() + oCfg.log_idx; itLog != oCfg.logs.end(); itLog++)
		{
			cnt++;
			if(cnt > get_max_num)
			{
				break;
			}
			oCfg.log_idx++;
			pLogText->add_text(*itLog);
		}
	}
	
}


#if 0
//too complex, simplify it
void GetLogByGnsEqually(SESSION_INFO &oInfo, ::pb::GetLogResp oPB)
{
	const size_t MAX_LOGS = 50;
	size_t gns_cnt = oInfo.cfgs.size();

	size_t total_logs_cnt = 0;

	std::vector<size_t> vec_loglen;
	for(auto it = oInfo.cfgs.begin(); it != oInfo.cfgs.end(); it++)
	{
		CFG oCfg = *it;
		vec_loglen.push_back(oCfg.logs.size() - oCfg.log_idx);
		total_logs_cnt += oCfg.logs.size() - oCfg.log_idx;
	}

	if(total_logs_cnt <= MAX_LOGS)
	{
		for(auto it = oInfo.cfgs.begin(); it != oInfo.cfgs.end(); it++)
		{
			CFG &oCfg = *it;	
			::pb::LogText* pLogText =  oPB.add_log_text();
			pLogText.set_gns_name(oCfg.gns_name);
			for(auto itLog = oCfg.logs.begin() + oCfg.log_idx; itLog != oCfg.logs.end(); itLog++)
			{
				oCfg.log_idx++;
				pLogText.add_text(*itLog);
			}
		}
	}
	else  //calc the allocation of MAX_LOGS for each gns
	{
		std::sort(vec_loglen.begin(), vec_loglen.end());
		int N = 0;
		size_t each_get = vec_loglen[0];
		size_t left_cnt = vec_loglen.size();
		int idx = 0;
		size_t total_each_get = 0;
		while(N < MAX_LOGS || idx >= vec_loglen.size() || left_cnt < 0)
		{
			N += each_get * left_cnt;
			idx++;
			each_get = vec_loglen[idx] - vec_loglen[idx-1];
			left_cnt--;
			total_each_get += each_get;
		}
		for(auto it = oInfo.cfgs.begin(); it != oInfo.cfgs.end(); it++)
		{
			CFG &oCfg = *it;	
			::pb::LogText* pLogText =  oPB.add_log_text();
			pLogText.set_gns_name(oCfg.gns_name);
			int cnt = 0;
			for(auto itLog = oCfg.logs.begin() + oCfg.log_idx; itLog != oCfg.logs.end(); itLog++)
			{
				cnt++;
				if(cnt > total_each_get)
				{
					break;
				}
				oCfg.log_idx++;
				pLogText.add_text(*itLog);
			}
		}
	}
}
#endif
