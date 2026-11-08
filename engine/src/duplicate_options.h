#pragma once

#include <string>

#include "duplicate_format.h"
#include "duplicate_mode.h"

struct DUPLICATE_OPTIONS
{
    bool enabled = false;
    DUPLICATE_FORMAT format = DUPLICATE_FORMAT::paren;
    std::u32string custom_text;
    DUPLICATE_MODE mode = DUPLICATE_MODE::keep_first;

    bool operator==(const DUPLICATE_OPTIONS &other) const = default;
};
