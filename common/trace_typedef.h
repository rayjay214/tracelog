#ifndef __TRACE_TYPEDEFINE_H__
#define __TRACE_TYPEDEFINE_H__

#include <vector>
#include <string>
#include <time.h>

struct MSG_HEAD
{
    int len;
    int id;
};

typedef struct tagCLIENT_INFO
{
	std::vector<std::string> gns_names;
	std::vector<std::string> ip_ports;
}CLIENT_INFO;

enum MSG_ID
{
    GNS_INFO_REQ = 0x88,  //app-->server
    GNS_INFO_RESP,        //server-->app
    CFG_INFO_REQ,         //server-->app
    LOGIN_REQ,            //cgi-->server
    LOGIN_RESP,           //server-->cgi
    SET_CONF_REQ,         //cgi-->server
    SET_CONF_RESP,        //server-->cgi
    LOG_INFO_REQ,         //app-->server
    GET_LOG_REQ,          //cgi-->server
    GET_LOG_RESP,         //server-->cgi
    HEARTBEAT_MSG,		  //all direction
    CLEAR_SESSION_REQ     //server-->app
};


typedef struct tagCFG
{
	std::string ip_port;  //reserved, related functionality will not be implemented this version
	std::string gns_name;
	int p1;
	long p2;
	std::string p3;
	std::vector<std::string> logs;
	int log_idx;  //element before log_idx means already got
}CFG;

enum errcode
{
	ERROR_OK = 0,
	ERROR_SESSION_NOT_FOUND,
	ERROR_SEND_FAILED,
	ERROR_PB_PARSE_FAILED,
	ERROR_PWD_NOT_MATCH,
	ERROR_CONN_NOT_EXIST
};

typedef struct tagSESSION_INFO
{
	std::vector<CFG> cfgs;
	time_t active_time;
}SESSION_INFO;


#endif //__TRACE_TYPEDEFINE_H__

