#include "win_text_service.h"

std::u32string WIN_TEXT_SERVICE::to_nfc(std::u32string_view text) const
{
    if (text.empty()) {
        return {};
    }
    const std::wstring source = to_wide(text);
    const int source_length = static_cast<int>(source.size());
    int capacity = NormalizeString(NormalizationC, source.c_str(), source_length, nullptr, 0);
    for (int attempt = 0; attempt < 8 && capacity > 0; ++attempt) {
        std::wstring buffer(static_cast<size_t>(capacity), L'\0');
        const int length = NormalizeString(NormalizationC, source.c_str(), source_length,
            buffer.data(), capacity);
        if (length > 0) {
            buffer.resize(static_cast<size_t>(length));
            return from_wide(buffer);
        }
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            break;
        }
        capacity = length < 0 ? -length : capacity * 2;
    }
    throw std::runtime_error("NormalizeString failed");
}

std::u32string WIN_TEXT_SERVICE::to_upper(std::u32string_view text) const
{
    return map_case(text, LCMAP_UPPERCASE | LCMAP_LINGUISTIC_CASING);
}

std::u32string WIN_TEXT_SERVICE::to_lower(std::u32string_view text) const
{
    return map_case(text, LCMAP_LOWERCASE | LCMAP_LINGUISTIC_CASING);
}

std::u32string WIN_TEXT_SERVICE::fold_key(std::u32string_view text) const
{
    return map_case(to_nfc(text), LCMAP_UPPERCASE);
}

bool WIN_TEXT_SERVICE::is_letter_or_digit(char32_t c) const
{
    const std::wstring units = to_wide(std::u32string_view(&c, 1));
    WORD types[2] = {};
    if (!GetStringTypeW(CT_CTYPE1, units.c_str(), static_cast<int>(units.size()), types)) {
        throw std::runtime_error("GetStringTypeW failed");
    }
    return (types[0] & (C1_ALPHA | C1_DIGIT)) != 0;
}

std::u32string WIN_TEXT_SERVICE::map_case(std::u32string_view text, DWORD flags)
{
    if (text.empty()) {
        return {};
    }
    const std::wstring source = to_wide(text);
    const int source_length = static_cast<int>(source.size());
    const int length = LCMapStringEx(LOCALE_NAME_INVARIANT, flags, source.c_str(), source_length,
        nullptr, 0, nullptr, nullptr, 0);
    if (length <= 0) {
        throw std::runtime_error("LCMapStringEx failed");
    }
    std::wstring buffer(static_cast<size_t>(length), L'\0');
    if (LCMapStringEx(LOCALE_NAME_INVARIANT, flags, source.c_str(), source_length, buffer.data(),
            length, nullptr, nullptr, 0) != length) {
        throw std::runtime_error("LCMapStringEx failed");
    }
    return from_wide(buffer);
}

std::wstring WIN_TEXT_SERVICE::to_wide(std::u32string_view text)
{
    const std::u16string units = UTF::encode_utf16(text);
    std::wstring result;
    result.reserve(units.size());
    for (const char16_t unit : units) {
        result.push_back(static_cast<wchar_t>(unit));
    }
    return result;
}

std::u32string WIN_TEXT_SERVICE::from_wide(std::wstring_view text)
{
    std::u16string units;
    units.reserve(text.size());
    for (const wchar_t unit : text) {
        units.push_back(static_cast<char16_t>(unit));
    }
    std::optional<std::u32string> result = UTF::decode_utf16(units);
    if (!result) {
        throw std::runtime_error("invalid UTF-16 from Windows API");
    }
    return std::move(*result);
}
