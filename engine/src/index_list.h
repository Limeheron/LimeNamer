#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "index_list_result.h"

class INDEX_LIST
{
public:
    static INDEX_LIST_RESULT parse(std::u32string_view text, uint32_t item_count);
    static std::u32string format(std::vector<uint32_t> indices);

private:
    static constexpr const size_t MIN_RANGE = 3;

    static bool is_space(char32_t c);
    static bool is_digit(char32_t c);
    static bool is_range_mark(char32_t c);
    static std::u32string number_text(uint32_t value);
};
