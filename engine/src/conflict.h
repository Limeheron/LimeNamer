#pragma once

#include <cstdint>

#include "conflict_kind.h"

struct CONFLICT
{
    CONFLICT_KIND kind = CONFLICT_KIND::none;
    uint64_t other_id = 0;

    bool operator==(const CONFLICT &other) const = default;
};
