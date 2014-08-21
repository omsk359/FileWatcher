#ifndef FILEMONITOR_H
#define	FILEMONITOR_H

#include <map>
#include <vector>
#include <string>
#include <ctime>
#include "boost/date_time/posix_time/posix_time.hpp"

class MonitorItem;

enum MonitorMode { HASH, MTIME };
enum FileState { FIRST, UNMODIFIED, MODIFIED, DELETED, ERROR_STATE };

class FileMonitor
{
    std::vector<MonitorItem *> files;
    static MonitorMode monMode;
    std::string stateFileName;
        
public:
    FileMonitor(MonitorMode monMode = HASH);
    static MonitorMode mode();
    int loadFileList(std::string fileName);
    int loadState();
    void saveState();
    std::map<FileState, int> checkState();
    virtual ~FileMonitor();

private:
    MonitorItem *findByName(std::string name);
};

class MonitorItem {
    std::string fileName;
    std::string hash;
//	std::time_t mtime;
	boost::posix_time::ptime mtime;
    
public:
    MonitorItem(std::string fName);
    std::string getName() { return fileName; }
    std::string storeState();
    void restoreState(std::string state);
    FileState checkState();
//    void fixState();
    
    bool isModified();
};

#endif	/* FILEMONITOR_H */

