#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "position.h"
#include "span.h"
#include "text_service.h"

class SPAN_FINDER
{
public:
    explicit SPAN_FINDER(const TEXT_SERVICE &text_service);

    std::vector<SPAN> find(const POSITION &position, std::u32string_view text) const;
    std::vector<SPAN> matches(std::u32string_view text, std::u32string_view needle,
        bool case_sensitive) const;

private:
    static constexpr const size_t ASCII_COUNT = 128;

    static bool is_insert(POSITION_TYPE type);
    static std::vector<SPAN> select(const std::vector<SPAN> &found, OCCURRENCE occurrence,
        uint32_t n);

    std::vector<SPAN> find_points(const POSITION &position, std::u32string_view text) const;
    std::vector<SPAN> find_ranges(const POSITION &position, std::u32string_view text) const;
    std::vector<SPAN> find_between(const POSITION &position, std::u32string_view text) const;
    std::vector<std::u32string> fold_each(std::u32string_view text) const;

    const TEXT_SERVICE &text_service_;
    std::array<std::u32string, ASCII_COUNT> ascii_folds_;
};
