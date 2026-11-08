#pragma once

#include <cstdint>

struct FILE_ID
{
    bool valid = false;
    uint64_t volume = 0;
    uint64_t high = 0;
    uint64_t low = 0;

    bool operator==(const FILE_ID &other) const = default;
};
