#pragma once

#include <cstdint>

struct NUMBER_SETTINGS
{
    int64_t start = 1;
    int64_t step = 1;
    uint32_t digits = 1;

    bool operator==(const NUMBER_SETTINGS &other) const = default;
};
