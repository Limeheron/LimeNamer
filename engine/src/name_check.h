#pragma once

#include <cstdint>

struct NAME_CHECK
{
    uint32_t errors = 0;
    uint32_t warnings = 0;

    bool operator==(const NAME_CHECK &other) const = default;
};
