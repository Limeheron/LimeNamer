#pragma once

#include <cstdint>
#include <string>

struct TOKEN_CONTEXT
{
    uint64_t index = 0;
    std::u32string original_name;
    std::u32string parent_name;
    bool is_dir = false;
    int64_t size = 0;
    int64_t modified_ms = 0;
    int64_t created_ms = 0;
};
