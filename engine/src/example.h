#pragma once

#include <cstdint>
#include <string>

#include "example_state.h"

struct EXAMPLE
{
    uint64_t item_id = 0;
    uint32_t index = 0;
    std::u32string before;
    std::u32string after;
    uint32_t prefix_len = 0;
    uint32_t suffix_len = 0;
    uint32_t errors = 0;
    EXAMPLE_STATE state = EXAMPLE_STATE::no_target;
};
