#pragma once

#include <cerrno>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "file_system.h"

class MAC_FILE_SYSTEM : public FILE_SYSTEM
{
public:
    FS_STATUS get_info(const std::filesystem::path &path, FILE_INFO &info) const override;
    FS_STATUS list_names(const std::filesystem::path &folder,
        std::vector<std::filesystem::path> &names) const override;
    FS_STATUS rename(const std::filesystem::path &from,
        const std::filesystem::path &to) override;

private:
    static FS_STATUS from_errno(int error);
    static int64_t to_ms(const timespec &time);
};
