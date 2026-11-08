#pragma once

#include "duplicate_options.h"

struct OPTIONS
{
    DUPLICATE_OPTIONS duplicate;

    bool operator==(const OPTIONS &other) const = default;
};
