#pragma once

#include <filesystem>
#include <stdexcept>

#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

class FILE_LOCK
{
public:
    explicit FILE_LOCK(const std::filesystem::path &path);
    ~FILE_LOCK();
    FILE_LOCK(const FILE_LOCK &) = delete;
    FILE_LOCK &operator=(const FILE_LOCK &) = delete;

private:
    std::filesystem::path path_;
#if defined(_WIN32)
    HANDLE handle_ = INVALID_HANDLE_VALUE;
#endif
};
