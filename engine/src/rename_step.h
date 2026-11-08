#pragma once

#include <cstdint>
#include <filesystem>

#include "step_kind.h"

struct RENAME_STEP
{
    uint64_t item_id = 0;
    std::filesystem::path from;
    std::filesystem::path to;
    STEP_KIND kind = STEP_KIND::direct;
};
