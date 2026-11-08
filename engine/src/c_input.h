#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "add_mode.h"
#include "limenamer.h"
#include "resolve_choice.h"
#include "sort_key.h"
#include "utf.h"

class C_INPUT
{
public:
    static std::optional<std::u32string> text(const char *value);
    static std::optional<std::filesystem::path> path(const char *value);
    static std::optional<std::vector<std::filesystem::path>> paths(const char *const *values,
        size_t count);
    static std::optional<std::vector<uint64_t>> ids(const uint64_t *values, size_t count);
    static std::optional<bool> flag(uint8_t value);

    template <typename ENUM>
    static std::optional<ENUM> choice(int32_t value, ENUM last)
    {
        if (value < 0 || value > static_cast<int32_t>(last)) {
            return std::nullopt;
        }
        return static_cast<ENUM>(value);
    }
};
