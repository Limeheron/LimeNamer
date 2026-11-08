#pragma once

#include <cstdint>

struct ADD_RESULT
{
    uint32_t added = 0;
    uint32_t skipped = 0;
    uint32_t failed = 0;

    bool operator==(const ADD_RESULT &other) const = default;
};
