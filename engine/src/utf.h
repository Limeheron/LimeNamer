#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

class UTF
{
public:
    static bool is_scalar(char32_t c);
    static bool is_valid(std::u32string_view text);

    static std::optional<std::u32string> decode_utf8(std::string_view text);
    static std::string encode_utf8(std::u32string_view text);
    static std::optional<std::u32string> decode_utf16(std::u16string_view text);
    static std::u16string encode_utf16(std::u32string_view text);

    static size_t utf8_length(std::u32string_view text);
    static size_t utf16_length(std::u32string_view text);

    static std::optional<std::filesystem::path> path_from_utf8(std::string_view text);
    static std::optional<std::string> path_to_utf8(const std::filesystem::path &path);
    static std::optional<std::u32string> path_to_u32(const std::filesystem::path &path);
    static std::filesystem::path path_from_u32(std::u32string_view text);
};
