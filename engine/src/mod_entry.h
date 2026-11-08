#pragma once

#include <cstdint>

#include "modification.h"

struct MOD_ENTRY
{
    uint64_t id = 0;
    MODIFICATION mod;
    bool enabled = true;

    bool operator==(const MOD_ENTRY &other) const = default;
};
