#pragma once

#include <string>
#include <string_view>

class TEXT_SERVICE
{
public:
    virtual ~TEXT_SERVICE() = default;

    virtual std::u32string to_nfc(std::u32string_view text) const = 0;
    virtual std::u32string to_upper(std::u32string_view text) const = 0;
    virtual std::u32string to_lower(std::u32string_view text) const = 0;
    virtual std::u32string fold_key(std::u32string_view text) const = 0;
    virtual bool is_letter_or_digit(char32_t c) const = 0;
};
