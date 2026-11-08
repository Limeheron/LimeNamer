#pragma once

#include "size_unit.h"

struct SIZE_SETTINGS
{
    SIZE_UNIT unit = SIZE_UNIT::automatic;

    bool operator==(const SIZE_SETTINGS &other) const = default;
};
