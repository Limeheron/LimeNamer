#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

#include "mod_part.h"

class NAME_PARTS
{
public:
    static NAME_PARTS split(std::u32string_view name, bool is_dir);

    bool applies(MOD_PART part) const;
    std::u32string text_of(MOD_PART part) const;
    std::u32string join(MOD_PART part, std::u32string_view text) const;

    const std::u32string &stem() const;
    const std::u32string &ext() const;
    bool has_ext() const;

private:
    NAME_PARTS(std::u32string stem, std::u32string ext, bool has_ext, bool is_dir);

    std::u32string whole() const;

    std::u32string stem_;
    std::u32string ext_;
    bool has_ext_ = false;
    bool is_dir_ = false;
};
