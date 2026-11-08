#pragma once

#include <filesystem>
#include <string>

#include "file_info.h"

struct FAKE_ENTRY
{
    std::filesystem::path path;
    std::u32string parent_key;
    FILE_INFO info;
};
