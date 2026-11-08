#pragma once

#include "sort_key.h"

struct SORT_STATE
{
    SORT_KEY key = SORT_KEY::added;
    bool descending = false;

    bool operator==(const SORT_STATE &other) const = default;
};
