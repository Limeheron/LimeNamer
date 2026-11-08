#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

struct CONFLICT_ENTRY
{
    uint64_t id = 0;
    std::filesystem::path folder;
    std::u32string current_name;
    std::u32string new_name;
    bool changed = false;
    bool is_dir = false;
};
