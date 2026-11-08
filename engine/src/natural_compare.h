#pragma once

#include <cstddef>
#include <string>
#include <string_view>

#include "text_service.h"

class NATURAL_COMPARE
{
public:
    explicit NATURAL_COMPARE(const TEXT_SERVICE &text_service);

    std::u32string key(std::u32string_view name) const;
    int compare(std::u32string_view a, std::u32string_view b, bool use_natural) const;

    static int compare_keys(std::u32string_view a, std::u32string_view b, bool use_natural);

private:
    static bool is_digit(char32_t c);
    static int compare_ordinal(std::u32string_view a, std::u32string_view b);
    static int compare_natural(std::u32string_view a, std::u32string_view b);

    const TEXT_SERVICE &text_service_;
};
