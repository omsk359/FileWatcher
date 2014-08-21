#include <iostream>
#include <string>
#include <vector>
#include <boost/program_options.hpp>

#include "FileMonitor.h"

namespace po = boost::program_options;
using namespace std;

int main(int argc, char** argv)
{
	// parse cmd line options
	po::options_description desc("Options");
	string inFile;
	desc.add_options()
		("help,h", "Show help")
		("input,i", po::value<std::string>(&inFile), "File containing a list of monitoring files")
		("type,t", po::value<std::string>(), "Monitor type: mtime, hash. Default: hash");

	po::variables_map vm;
	std::string monitorType = "hash";
	try {
		po::parsed_options parsed = po::command_line_parser(argc, argv).options(desc).allow_unregistered().run();
		po::store(parsed, vm);
		po::notify(vm);
		if (vm.count("help") || !vm.count("input")) {
			std::cout << desc << std::endl;
			return 0;
		}
		if (vm.count("type")) {
			monitorType = vm["type"].as<std::string>();
			if (monitorType != "mtime" && monitorType != "hash") {
				cout << desc << endl;
				return 0;
			}
		}
	}
	catch (std::exception& ex) {
		std::cout << desc << std::endl;
		return 0;
	}
	MonitorMode mode;
	if (monitorType == "mtime")
		mode = MTIME;
	else
		mode = HASH;

	// run monitor
	FileMonitor mon(mode);
	cout << "Read list of watching files...\n";
	int n = mon.loadFileList(inFile);
    cout << "Total watched files: " << n << endl;
	cout << "----------------------------------\n";
	cout << "Load previous state...\n";
	n = mon.loadState();
    cout << "Total state files: " << n << endl;
	cout << "----------------------------------\n";
	cout << "Checking state...\n";
	map<FileState, int> states = mon.checkState();
	cout << "Total state:\nmodified - " << states[MODIFIED] << "; unmodifued - " <<
		states[UNMODIFIED] << "; deleted - " << states[DELETED] << "; new - " << states[FIRST] << ";\n";
	mon.saveState();
	system("pause");
    return 0;
}

