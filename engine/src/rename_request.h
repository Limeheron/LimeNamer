#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

struct RENAME_REQUEST
{
    uint64_t item_id = 0;
    std::filesystem::path path;
    std::u32string name;
    std::u32string new_name;
};
