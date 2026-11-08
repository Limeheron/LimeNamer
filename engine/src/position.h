#pragma once

#include <cstdint>
#include <string>

#include "occurrence.h"
#include "position_type.h"

struct POSITION
{
    POSITION_TYPE type = POSITION_TYPE::start;
    uint32_t n = 0;
    uint32_t count = 0;
    std::u32string text;
    bool case_sensitive = false;
    OCCURRENCE occurrence = OCCURRENCE::first;
    uint32_t occurrence_n = 1;
    std::u32string start_text;
    bool from_beginning = false;
    std::u32string end_text;
    bool to_end = false;
    bool include_delimiters = false;

    bool operator==(const POSITION &other) const = default;
};
