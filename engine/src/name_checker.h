#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "name_check.h"
#include "name_issue.h"
#include "utf.h"

class NAME_CHECKER
{
public:
    static constexpr const size_t MAX_NAME_UNITS = 255;
    static constexpr const size_t MAX_PATH_UNITS = 259;

    explicit NAME_CHECKER(bool windows_host);

    NAME_CHECK check(std::u32string_view name, std::u32string_view current_name,
        size_t folder_units) const;
    uint32_t issues(std::u32string_view name, size_t folder_units) const;

    static size_t folder_units(std::u32string_view folder);

private:
    static bool is_blank(std::u32string_view name);
    static bool is_reserved(std::u32string_view name);

    bool windows_host_ = false;
};
