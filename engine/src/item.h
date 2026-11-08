#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include "file_info.h"

struct ITEM
{
    uint64_t id = 0;
    std::filesystem::path path;
    std::filesystem::path folder;
    std::u32string name;
    std::u32string key;
    FILE_INFO info;
};
