#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <system_error>

#include "file_system.h"

#if defined(_WIN32)
#include "win/win_file_system.h"
#else
#include "mac/mac_file_system.h"
#endif

class REAL_FS_ENV
{
public:
    REAL_FS_ENV();
    ~REAL_FS_ENV();
    REAL_FS_ENV(const REAL_FS_ENV &) = delete;
    REAL_FS_ENV &operator=(const REAL_FS_ENV &) = delete;

    FILE_SYSTEM &file_system();
    const std::filesystem::path &root() const;
    void make_file(const std::filesystem::path &path, int64_t size);
    void make_folder(const std::filesystem::path &path);

private:
    std::unique_ptr<FILE_SYSTEM> file_system_;
    std::filesystem::path root_;
};
