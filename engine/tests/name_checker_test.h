#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "name_check.h"
#include "name_checker.h"
#include "name_issue.h"
#include "utf.h"

class NAME_CHECKER_TEST : public testing::Test
{
protected:
    static constexpr const size_t FOLDER = 20;

    NAME_CHECK on_windows(std::u32string_view name, std::u32string_view current = U"old.txt")
        const
    {
        return windows_.check(name, current, FOLDER);
    }

    NAME_CHECK on_mac(std::u32string_view name, std::u32string_view current = U"old.txt") const
    {
        return mac_.check(name, current, FOLDER);
    }

    static NAME_CHECK result(uint32_t errors, uint32_t warnings)
    {
        NAME_CHECK value;
        value.errors = errors;
        value.warnings = warnings;
        return value;
    }

    NAME_CHECKER windows_{true};
    NAME_CHECKER mac_{false};
};
