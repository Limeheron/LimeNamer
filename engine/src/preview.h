#pragma once

#include <cstdint>
#include <vector>

#include "preview_row.h"

struct PREVIEW
{
    std::vector<PREVIEW_ROW> rows;
    uint32_t changed = 0;
    uint32_t runnable = 0;
    uint32_t warning = 0;
    uint32_t error = 0;
    uint32_t conflict = 0;
};
