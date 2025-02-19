
#include "sysutil.hxx"

#include <cstring>
#include <errno.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>

namespace nuraft {

void ensure_dir(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFDIR) != 0) {
        return;
    }
#ifdef _WIN32
    int result = _mkdir(path.c_str());
#else
    int result = mkdir(path.c_str(), 0777);
#endif
    if (result != 0) {
        std::cerr << "Error creating directory " << path << ": " << std::strerror(errno) << std::endl;
        exit(1);
    }
}

std::string concat_path(const std::string& dir, const std::string& file) {
    std::string result = dir;

    // Ensure path1 doesn't end with a slash, unless it's the root path
    if (!result.empty() && result.back() == '/') {
        result.pop_back();
    }

    // Ensure path2 doesn't start with a slash
    std::string path2_modified = file;
    if (!path2_modified.empty() && path2_modified.front() == '/') {
        path2_modified.erase(0, 1);
    }

    if (!path2_modified.empty()) {
        result += "/" + path2_modified;
    }
    return result;
}
}; // namespace nuraft
