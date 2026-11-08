#pragma once

#include <filesystem>
#include <string>
#include <unordered_set>

struct FOLDER_NAMES
{
    std::filesystem::path folder;
    std::unordered_set<std::u32string> keys;
    bool readable = false;
};
