#include "FileMonitor.h"
#include "FileAPI.h"

#include <boost/filesystem/operations.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>

namespace fs = boost::filesystem;
namespace pt = boost::posix_time;
using namespace std;

MonitorMode FileMonitor::monMode;

#define MAX_TIME_STRING_LENGTH 19

FileMonitor::FileMonitor(MonitorMode monMode)
{
    stateFileName = "watchState.txt";
    this->monMode = monMode;
}

FileMonitor::~FileMonitor()
{
	for (size_t i = 0; i < files.size(); i++)
        delete files[i];
}

MonitorMode FileMonitor::mode() { return monMode; }

int FileMonitor::loadFileList(string fileName)
{
    ifstream in(fileName.c_str());
    int cnt = 0;
    for (string line; getline(in, line); cnt++) {
        fs::path p(line);
        fs::path full_p = fs::complete(p);
        cout << "read line: " << line << endl;
        cout << "full path: " << full_p.string() << endl;
        MonitorItem *item = new MonitorItem(full_p.string());
        files.push_back(item);
    }
    return cnt;
}

int FileMonitor::loadState()
{
    ifstream in(stateFileName.c_str());;
    int cnt = 0;
    for (string line; getline(in, line); cnt++) {
        cout << "line: " << line << endl;
        size_t pos = line.find('\t');
        if (pos == string::npos || !pos) {
			cout << "loadState failed parse line: " << line << endl;
            return -1;
        }
        string name = line.substr(0, pos);
        string state = line.substr(pos + 1);
        MonitorItem *item = findByName(name);
        if (!item) {
            cout << "Found state for not watched file: " << name << endl;
            continue;
        }
        item->restoreState(state);
        cout << "read state for '" << name << "' - '" << state << "'\n"; 
    }
    return cnt;
}

void FileMonitor::saveState()
{
    ofstream out(stateFileName.c_str());
	for (size_t i = 0; i < files.size(); i++) {
        MonitorItem *item = files[i];
        out << item->getName() << '\t' << item->storeState() << endl;
    }
}

map<FileState, int> FileMonitor::checkState()
{
    map<FileState, int> result;
	for (size_t i = 0; i < files.size(); i++) {
        FileState state = files[i]->checkState();
        string stateStr;
        switch (state) {
            case UNMODIFIED:
                stateStr = "not modified";
                break;
			case MODIFIED:
                stateStr = "modified";
                break;
			case DELETED:
                stateStr = "file not found";
                break;
			case FIRST:
                stateStr = "add to the Watcher";
                break;
            default:
                stateStr = "error";
        }
		cout << files[i]->getName() << " -> " << stateStr << endl;
        result[state]++;
    }
    return result;
}

MonitorItem *FileMonitor::findByName(string name)
{
    for (size_t i = 0; i < files.size(); i++)
        if (files[i]->getName() == name)
            return files[i];
    return 0;
}

MonitorItem::MonitorItem(std::string fName) : fileName(fName), mtime(pt::neg_infin)
{}

string MonitorItem::storeState()
{
    if (FileMonitor::mode() == HASH)
        return hash;
	if (FileMonitor::mode() == MTIME) {
//		pt::ptime time = pt::from_time_t(mtime);
		return pt::to_iso_string(mtime);
	}
	return "";
}

void MonitorItem::restoreState(std::string state)
{
	if (FileMonitor::mode() == HASH)
		hash = state;
	else if (FileMonitor::mode() == MTIME) {
		try {
			mtime = pt::from_iso_string(state);
//			pt::ptime t(pt::from_iso_string(state));
//			tm pt_tm = to_tm(t);
//			mtime = mktime(&pt_tm);
			cout << "restore time for " << fileName << " is: " << mtime << endl;
		}
		catch (boost::exception const& ex) {
			cout << "Exception\n";
		}
	}
}

FileState MonitorItem::checkState()
{
    if (FileMonitor::mode() == HASH) {
        unsigned result = 0;
        string newHash = FileAPI::md5(fileName, result);
        if (!result)
    //            printf("Hash of '%s' is: %s\n", test, hash);
            cout << "Hash of '" << fileName << "' is: '" << newHash << "'\n";
        else {
            if (result == 2)
                return DELETED;
			cout << "Failed! Result: " << result << endl;
            return ERROR_STATE;
        }
		if (hash.empty()) {
			hash = newHash;
			return FIRST;
		}
		return newHash != hash ? MODIFIED : UNMODIFIED;
	} else if (FileMonitor::mode() == MTIME) {
		fs::path p(fileName);
		if (!fs::exists(p))
			return DELETED;
		time_t newMtime_t = fs::last_write_time(p);
		pt::ptime newMtime = pt::from_time_t(newMtime_t);
		cout << "Last modify time of " << fileName << " is: " << newMtime << endl;
		if (mtime.is_neg_infinity()) {
			mtime = newMtime;
			return FIRST;
		}
		return newMtime != mtime ? MODIFIED : UNMODIFIED;
	}
	return ERROR_STATE;
}
