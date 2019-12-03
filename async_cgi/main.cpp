#include "AsyncCommThread.h"
#include "gmapp.h"
#include "Log.h"
#include "helper.h"
#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>

using namespace muduo::net;
namespace po = boost::program_options;

log4cxx::LoggerPtr g_logger = log4cxx::Logger::getRootLogger();
Config g_conf;

void read_settings(po::options_description& desc,
                   po::variables_map& vm)
{
  std::ifstream settings_file("../etc/cgi.ini");

  // Clear the map.
  vm = po::variables_map();

  po::store(po::parse_config_file(settings_file , desc), vm);
  po::notify(vm);
}

int main(int argc, char* argv[])
{
    log4cxx::PropertyConfigurator::configure("../etc/log4cxx.cfg");

	po::options_description desc("Options");
	desc.add_options()
		("TraceServerIp", po::value<std::string>(&g_conf.traceServerIp), "TraceServerIp");
	desc.add_options()
		("TraceServerPort", po::value<int>(&g_conf.traceServerPort), "TraceServerPort");
	desc.add_options()
		("ListenPort", po::value<int>(&g_conf.listenPort), "ListenPort");
	po::variables_map vm;

	read_settings(desc, vm);

	CGMApp app;

    app.g_pLogger = g_logger;    
    app.SetOpenRunLog();

    AsyncCommThread commthr;

    app.AddThread(&commthr, "COMM", 0, 3);

    app.Run("0.0.0.0", g_conf.listenPort);
}
