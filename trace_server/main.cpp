#include "gmapp.h"
#include "AsyncCommThread.h"
#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>
#include "helper.h"

log4cxx::LoggerPtr g_logger = log4cxx::Logger::getRootLogger();
std::string g_pass_encrypt;
std::map<int, CLIENT_INFO> g_map_fd_cli_info;  //保存所有TraceClient信息  
std::map<std::string, SESSION_INFO> g_map_session_info;

Config g_conf;

namespace po = boost::program_options;

void read_settings(po::options_description& desc,
                   po::variables_map& vm)
{
  std::ifstream settings_file("../etc/trace_server.ini");

  // Clear the map.
  vm = po::variables_map();

  po::store(po::parse_config_file(settings_file , desc), vm);
  po::notify(vm);
}


void init()
{
	log4cxx::PropertyConfigurator::configure("../etc/log4cxx.cfg");

	po::options_description desc("Options");
	desc.add_options()
		("Password", po::value<std::string>(&g_conf.password), "Password");
	desc.add_options()
		("MaxGetLogs", po::value<int>(&g_conf.maxGetLogs), "MaxGetLogs");
	desc.add_options()
		("MaxPerserveLogs", po::value<int>(&g_conf.maxPerserveLogs), "MaxPerserveLogs");
	desc.add_options()
		("SessionExpireSeconds", po::value<int>(&g_conf.sessionExpireSeconds), "SessionExpireSeconds");
	desc.add_options()
		("ListenPort", po::value<int>(&g_conf.listenPort), "ListenPort");
		
	po::variables_map vm;

	read_settings(desc, vm);

	MakeMD5(g_conf.password, g_pass_encrypt);
}

int main(int argc, char* argv[])
{
    init();

	CGMApp app;
    
    app.SetOpenRunLog();

    AsyncCommThread commthr;

    app.AddThread(&commthr, "AsyncComm");

    app.Run("0.0.0.0", g_conf.listenPort);
}

