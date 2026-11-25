#pragma once

#include <filesystem>

#include "file_info.h"
#include "fs_status.h"

struct FOLDER_ENTRY
{
    std::filesystem::path name;
    FILE_INFO info;
    FS_STATUS status;
};
