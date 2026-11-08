#include "utf.h"

bool UTF::is_scalar(char32_t c)
{
    return c <= 0x10FFFFu && (c < 0xD800u || c > 0xDFFFu);
}

bool UTF::is_valid(std::u32string_view text)
{
    for (const char32_t c : text) {
        if (!is_scalar(c)) {
            return false;
        }
    }
    return true;
}

std::optional<std::u32string> UTF::decode_utf8(std::string_view text)
{
    std::u32string result;
    result.reserve(text.size());
    size_t i = 0;
    while (i < text.size()) {
        const char32_t lead = static_cast<unsigned char>(text[i]);
        if (lead < 0x80u) {
            result.push_back(lead);
            ++i;
            continue;
        }
        size_t length = 0;
        char32_t c = 0;
        char32_t minimum = 0;
        if ((lead & 0xE0u) == 0xC0u) {
            length = 2;
            c = lead & 0x1Fu;
            minimum = 0x80u;
        } else if ((lead & 0xF0u) == 0xE0u) {
            length = 3;
            c = lead & 0x0Fu;
            minimum = 0x800u;
        } else if ((lead & 0xF8u) == 0xF0u) {
            length = 4;
            c = lead & 0x07u;
            minimum = 0x10000u;
        } else {
            return std::nullopt;
        }
        if (length > text.size() - i) {
            return std::nullopt;
        }
        for (size_t k = 1; k < length; ++k) {
            const char32_t next = static_cast<unsigned char>(text[i + k]);
            if ((next & 0xC0u) != 0x80u) {
                return std::nullopt;
            }
            c = (c << 6) | (next & 0x3Fu);
        }
        if (c < minimum || !is_scalar(c)) {
            return std::nullopt;
        }
        result.push_back(c);
        i += length;
    }
    return result;
}

std::string UTF::encode_utf8(std::u32string_view text)
{
    std::string result;
    result.reserve(text.size());
    for (const char32_t c : text) {
        if (!is_scalar(c)) {
            throw std::invalid_argument("invalid code point");
        }
        if (c < 0x80u) {
            result.push_back(static_cast<char>(c));
        } else if (c < 0x800u) {
            result.push_back(static_cast<char>(0xC0u | (c >> 6)));
            result.push_back(static_cast<char>(0x80u | (c & 0x3Fu)));
        } else if (c < 0x10000u) {
            result.push_back(static_cast<char>(0xE0u | (c >> 12)));
            result.push_back(static_cast<char>(0x80u | ((c >> 6) & 0x3Fu)));
            result.push_back(static_cast<char>(0x80u | (c & 0x3Fu)));
        } else {
            result.push_back(static_cast<char>(0xF0u | (c >> 18)));
            result.push_back(static_cast<char>(0x80u | ((c >> 12) & 0x3Fu)));
            result.push_back(static_cast<char>(0x80u | ((c >> 6) & 0x3Fu)));
            result.push_back(static_cast<char>(0x80u | (c & 0x3Fu)));
        }
    }
    return result;
}

std::optional<std::u32string> UTF::decode_utf16(std::u16string_view text)
{
    std::u32string result;
    result.reserve(text.size());
    for (size_t i = 0; i < text.size(); ++i) {
        const char32_t unit = text[i];
        if (unit < 0xD800u || unit > 0xDFFFu) {
            result.push_back(unit);
            continue;
        }
        if (unit > 0xDBFFu || i + 1 >= text.size()) {
            return std::nullopt;
        }
        const char32_t low = text[i + 1];
        if (low < 0xDC00u || low > 0xDFFFu) {
            return std::nullopt;
        }
        result.push_back(0x10000u + ((unit - 0xD800u) << 10) + (low - 0xDC00u));
        ++i;
    }
    return result;
}

std::u16string UTF::encode_utf16(std::u32string_view text)
{
    std::u16string result;
    result.reserve(text.size());
    for (const char32_t c : text) {
        if (!is_scalar(c)) {
            throw std::invalid_argument("invalid code point");
        }
        if (c < 0x10000u) {
            result.push_back(static_cast<char16_t>(c));
        } else {
            const char32_t offset = c - 0x10000u;
            result.push_back(static_cast<char16_t>(0xD800u + (offset >> 10)));
            result.push_back(static_cast<char16_t>(0xDC00u + (offset & 0x3FFu)));
        }
    }
    return result;
}

size_t UTF::utf8_length(std::u32string_view text)
{
    size_t length = 0;
    for (const char32_t c : text) {
        if (c < 0x80u) {
            length += 1;
        } else if (c < 0x800u) {
            length += 2;
        } else if (c < 0x10000u) {
            length += 3;
        } else {
            length += 4;
        }
    }
    return length;
}

size_t UTF::utf16_length(std::u32string_view text)
{
    size_t length = 0;
    for (const char32_t c : text) {
        length += c < 0x10000u ? 1 : 2;
    }
    return length;
}

std::optional<std::filesystem::path> UTF::path_from_utf8(std::string_view text)
{
    const std::optional<std::u32string> decoded = decode_utf8(text);
    if (!decoded) {
        return std::nullopt;
    }
    return path_from_u32(*decoded);
}

std::optional<std::string> UTF::path_to_utf8(const std::filesystem::path &path)
{
    const std::optional<std::u32string> decoded = path_to_u32(path);
    if (!decoded) {
        return std::nullopt;
    }
    return encode_utf8(*decoded);
}

std::optional<std::u32string> UTF::path_to_u32(const std::filesystem::path &path)
{
#if defined(_WIN32)
    const std::wstring &native = path.native();
    std::u16string units;
    units.reserve(native.size());
    for (const wchar_t unit : native) {
        units.push_back(static_cast<char16_t>(unit));
    }
    return decode_utf16(units);
#else
    return decode_utf8(path.native());
#endif
}

std::filesystem::path UTF::path_from_u32(std::u32string_view text)
{
#if defined(_WIN32)
    const std::u16string units = encode_utf16(text);
    std::wstring native;
    native.reserve(units.size());
    for (const char16_t unit : units) {
        native.push_back(static_cast<wchar_t>(unit));
    }
    return std::filesystem::path(std::move(native));
#else
    return std::filesystem::path(encode_utf8(text));
#endif
}
