#pragma once

#include <cstdint>
#include <filesystem>

#include "rename_failure.h"
#include "report_stage.h"

struct REPORT_ITEM
{
    uint64_t item_id = 0;
    std::filesystem::path from;
    std::filesystem::path to;
    std::filesystem::path current;
    REPORT_STAGE stage = REPORT_STAGE::run;
    RENAME_FAILURE failure = RENAME_FAILURE::none;
    int32_t os_error = 0;
};
