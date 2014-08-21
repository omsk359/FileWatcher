#ifndef FILEHASH_H
#define	FILEHASH_H

#include <string>
#include <ctime>

class FileAPI {

//	static std::time_t filetime_to_timet(FILETIME const& ft);

public:
    static std::string md5(std::string fileName, unsigned &status);
//	static std::time_t modifyTime(std::string fileName, unsigned &status);
};

#endif	/* FILEHASH_H */

