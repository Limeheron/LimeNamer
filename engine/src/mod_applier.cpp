#include "mod_applier.h"

MOD_APPLIER::MOD_APPLIER(const TEXT_SERVICE &text_service)
    : text_service_(text_service),
      finder_(text_service)
{
}

std::u32string MOD_APPLIER::apply(const MODIFICATION &mod, std::u32string_view name,
    bool is_dir, std::u32string_view value) const
{
    const NAME_PARTS parts = NAME_PARTS::split(name, is_dir);
    if (!parts.applies(mod.part)) {
        return std::u32string(name);
    }
    const std::u32string text = parts.text_of(mod.part);
    const std::vector<SPAN> spans = finder_.find(mod.position, text);
    if (spans.empty()) {
        return std::u32string(name);
    }
    const std::u32string edited = edit(mod, text, spans, value);
    if (edited == text) {
        return std::u32string(name);
    }
    return parts.join(mod.part, text_service_.to_nfc(edited));
}

bool MOD_APPLIER::is_ascii_digit(char32_t c)
{
    return c >= U'0' && c <= U'9';
}

bool MOD_APPLIER::is_apostrophe(char32_t c)
{
    return c == U'\'' || c == U'’';
}

void MOD_APPLIER::pad_digits(std::u32string &result, std::u32string_view text, const SPAN &span,
    uint32_t digits)
{
    size_t at = span.begin;
    if (at > 0 && at < text.size() && is_ascii_digit(text[at - 1])) {
        while (at < span.end && is_ascii_digit(text[at])) {
            ++at;
        }
    }
    std::vector<std::pair<size_t, size_t>> pads;
    while (at < span.end) {
        if (!is_ascii_digit(text[at])) {
            ++at;
            continue;
        }
        size_t end = at;
        while (end < text.size() && is_ascii_digit(text[end])) {
            ++end;
        }
        if (end - at < digits) {
            pads.emplace_back(at, digits - (end - at));
        }
        at = end;
    }
    for (auto pad = pads.rbegin(); pad != pads.rend(); ++pad) {
        result.insert(pad->first, pad->second, U'0');
    }
}

std::u32string MOD_APPLIER::edit(const MODIFICATION &mod, std::u32string_view text,
    const std::vector<SPAN> &spans, std::u32string_view value) const
{
    std::u32string result(text);
    for (auto span = spans.rbegin(); span != spans.rend(); ++span) {
        const size_t length = span->end - span->begin;
        switch (mod.kind) {
        case MOD_KIND::insert:
            result.insert(span->begin, value);
            break;
        case MOD_KIND::replace:
            result.replace(span->begin, length, value);
            break;
        case MOD_KIND::remove:
            result.erase(span->begin, length);
            break;
        case MOD_KIND::format:
            if (mod.format.type == FORMAT_TYPE::pad_digits) {
                pad_digits(result, text, *span, mod.format.digits);
            } else {
                result.replace(span->begin, length,
                    format(text.substr(span->begin, length), mod.format));
            }
            break;
        }
    }
    return result;
}

std::u32string MOD_APPLIER::format(std::u32string_view text,
    const FORMAT_SETTINGS &settings) const
{
    switch (settings.type) {
    case FORMAT_TYPE::upper:
        return text_service_.to_upper(text);
    case FORMAT_TYPE::lower:
        return text_service_.to_lower(text);
    case FORMAT_TYPE::title:
        return capitalize(text, true);
    case FORMAT_TYPE::sentence:
        return capitalize(text, false);
    case FORMAT_TYPE::pad_digits:
        break;
    }
    return std::u32string(text);
}

std::u32string MOD_APPLIER::capitalize(std::u32string_view text, bool every_word) const
{
    std::u32string result;
    result.reserve(text.size());
    bool word_start = true;
    bool capitalized = false;
    for (const char32_t c : text) {
        const bool word_char = is_letter_or_digit(c);
        if (word_char && word_start && (every_word || !capitalized)) {
            result += upper(c);
            capitalized = true;
        } else {
            result += lower(c);
        }
        if (word_char) {
            word_start = false;
        } else if (!is_apostrophe(c)) {
            word_start = true;
        }
    }
    return result;
}

std::u32string MOD_APPLIER::upper(char32_t c) const
{
    if (c < 0x80u) {
        const char32_t mapped = c >= U'a' && c <= U'z' ? c - U'a' + U'A' : c;
        return std::u32string(1, mapped);
    }
    return text_service_.to_upper(std::u32string_view(&c, 1));
}

std::u32string MOD_APPLIER::lower(char32_t c) const
{
    if (c < 0x80u) {
        const char32_t mapped = c >= U'A' && c <= U'Z' ? c - U'A' + U'a' : c;
        return std::u32string(1, mapped);
    }
    return text_service_.to_lower(std::u32string_view(&c, 1));
}

bool MOD_APPLIER::is_letter_or_digit(char32_t c) const
{
    if (c < 0x80u) {
        return (c >= U'a' && c <= U'z') || (c >= U'A' && c <= U'Z') || is_ascii_digit(c);
    }
    return text_service_.is_letter_or_digit(c);
}
