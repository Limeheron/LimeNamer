#pragma once

#include <filesystem>
#include <vector>

#include "file_info.h"
#include "fs_status.h"

class FILE_SYSTEM
{
public:
    virtual ~FILE_SYSTEM() = default;

    virtual FS_STATUS get_info(const std::filesystem::path &path, FILE_INFO &info) const = 0;
    virtual FS_STATUS list_names(const std::filesystem::path &folder,
        std::vector<std::filesystem::path> &names) const = 0;
    virtual FS_STATUS rename(const std::filesystem::path &from,
        const std::filesystem::path &to) = 0;
};
