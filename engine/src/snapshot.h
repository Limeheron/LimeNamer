#pragma once

#include <cstdint>
#include <vector>

#include "mod_entry.h"
#include "options.h"
#include "sort_state.h"

struct SNAPSHOT
{
    std::vector<uint64_t> order;
    SORT_STATE sort;
    std::vector<MOD_ENTRY> mods;
    OPTIONS options;

    bool operator==(const SNAPSHOT &other) const = default;
};
