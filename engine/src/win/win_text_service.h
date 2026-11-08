#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include <windows.h>

#include "text_service.h"
#include "utf.h"

class WIN_TEXT_SERVICE : public TEXT_SERVICE
{
public:
    std::u32string to_nfc(std::u32string_view text) const override;
    std::u32string to_upper(std::u32string_view text) const override;
    std::u32string to_lower(std::u32string_view text) const override;
    std::u32string fold_key(std::u32string_view text) const override;
    bool is_letter_or_digit(char32_t c) const override;

private:
    static std::u32string map_case(std::u32string_view text, DWORD flags);
    static std::wstring to_wide(std::u32string_view text);
    static std::u32string from_wide(std::wstring_view text);
};
