#pragma once

#include <bit>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

#include <CoreFoundation/CoreFoundation.h>

#include "text_service.h"

class MAC_TEXT_SERVICE : public TEXT_SERVICE
{
public:
    std::u32string to_nfc(std::u32string_view text) const override;
    std::u32string to_upper(std::u32string_view text) const override;
    std::u32string to_lower(std::u32string_view text) const override;
    std::u32string fold_key(std::u32string_view text) const override;
    bool is_letter_or_digit(char32_t c) const override;

private:
    using CF_STRING =
        std::unique_ptr<std::remove_pointer_t<CFMutableStringRef>, decltype(&CFRelease)>;

    static CF_STRING to_cf(std::u32string_view text);
    static std::u32string from_cf(CFStringRef text);
};
