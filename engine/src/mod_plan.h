#pragma once

#include <cstdint>
#include <unordered_map>

#include "modification.h"

struct MOD_PLAN
{
    const MODIFICATION *mod = nullptr;
    std::unordered_map<uint64_t, uint64_t> indexes;
};
