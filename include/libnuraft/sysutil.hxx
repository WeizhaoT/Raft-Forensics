
#ifndef _SYSUTIL_HXX_
#define _SYSUTIL_HXX_

#include <cstring>
#include <errno.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>

namespace nuraft {

void ensure_dir(const std::string& path);

std::string concat_path(const std::string& dir, const std::string& file);

}; // namespace nuraft

#endif