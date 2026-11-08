#pragma once

#include <cstddef>

struct SPAN
{
    size_t begin = 0;
    size_t end = 0;

    bool operator==(const SPAN &other) const = default;
};
