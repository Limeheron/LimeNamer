#pragma once

#include <cstdint>
#include <filesystem>

#include "file_info.h"

struct RECORD_ENTRY
{
    uint64_t item_id = 0;
    std::filesystem::path before;
    std::filesystem::path after;
    FILE_INFO info;
};
