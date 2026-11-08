#pragma once

#include <string>

struct DATE_SETTINGS
{
    std::u32string format = U"yyyyMMdd";

    bool operator==(const DATE_SETTINGS &other) const = default;
};
