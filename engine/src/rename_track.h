#pragma once

#include <cstdint>
#include <filesystem>

#include "rename_failure.h"

struct RENAME_TRACK
{
    uint64_t item_id = 0;
    std::filesystem::path original;
    std::filesystem::path target;
    std::filesystem::path current;
    bool is_dir = false;
    bool failed = false;
    bool reverted = false;
    RENAME_FAILURE failure = RENAME_FAILURE::none;
    int32_t os_error = 0;
};
