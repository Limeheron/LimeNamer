#pragma once

#include <cstdint>

#include "format_type.h"

struct FORMAT_SETTINGS
{
    FORMAT_TYPE type = FORMAT_TYPE::upper;
    uint32_t digits = 1;

    bool operator==(const FORMAT_SETTINGS &other) const = default;
};
