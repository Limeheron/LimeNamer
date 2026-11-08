#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "format_settings.h"
#include "modification.h"
#include "name_parts.h"
#include "span.h"
#include "span_finder.h"
#include "text_service.h"

class MOD_APPLIER
{
public:
    explicit MOD_APPLIER(const TEXT_SERVICE &text_service);

    std::u32string apply(const MODIFICATION &mod, std::u32string_view name, bool is_dir,
        std::u32string_view value) const;

private:
    static bool is_ascii_digit(char32_t c);
    static bool is_apostrophe(char32_t c);
    static void pad_digits(std::u32string &result, std::u32string_view text, const SPAN &span,
        uint32_t digits);

    std::u32string edit(const MODIFICATION &mod, std::u32string_view text,
        const std::vector<SPAN> &spans, std::u32string_view value) const;
    std::u32string format(std::u32string_view text, const FORMAT_SETTINGS &settings) const;
    std::u32string capitalize(std::u32string_view text, bool every_word) const;
    std::u32string upper(char32_t c) const;
    std::u32string lower(char32_t c) const;
    bool is_letter_or_digit(char32_t c) const;

    const TEXT_SERVICE &text_service_;
    SPAN_FINDER finder_;
};
