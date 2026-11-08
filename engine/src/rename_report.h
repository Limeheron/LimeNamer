#pragma once

#include <cstdint>
#include <vector>

#include "report_item.h"

struct RENAME_REPORT
{
    std::vector<REPORT_ITEM> problems;
    uint32_t total = 0;
    uint32_t succeeded = 0;
    uint32_t skipped = 0;
    uint32_t failed = 0;
    bool needs_choice = false;
};
