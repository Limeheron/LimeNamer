#include "name_parts.h"

NAME_PARTS NAME_PARTS::split(std::u32string_view name, bool is_dir)
{
    if (is_dir) {
        return NAME_PARTS(std::u32string(name), {}, false, true);
    }
    const size_t first = name.find_first_not_of(U'.');
    if (first == std::u32string_view::npos) {
        return NAME_PARTS(std::u32string(name), {}, false, false);
    }
    const size_t dot = name.rfind(U'.');
    if (dot == std::u32string_view::npos || dot < first || dot + 1 == name.size()) {
        return NAME_PARTS(std::u32string(name), {}, false, false);
    }
    return NAME_PARTS(std::u32string(name.substr(0, dot)), std::u32string(name.substr(dot + 1)),
        true, false);
}

bool NAME_PARTS::applies(MOD_PART part) const
{
    return part != MOD_PART::ext || !is_dir_;
}

std::u32string NAME_PARTS::text_of(MOD_PART part) const
{
    switch (part) {
    case MOD_PART::name:
        return stem_;
    case MOD_PART::ext:
        return ext_;
    case MOD_PART::full:
        return whole();
    }
    return whole();
}

std::u32string NAME_PARTS::join(MOD_PART part, std::u32string_view text) const
{
    switch (part) {
    case MOD_PART::name:
        return has_ext_ ? std::u32string(text) + U'.' + ext_ : std::u32string(text);
    case MOD_PART::ext:
        if (is_dir_) {
            return whole();
        }
        return text.empty() ? stem_ : stem_ + U'.' + std::u32string(text);
    case MOD_PART::full:
        return std::u32string(text);
    }
    return std::u32string(text);
}

const std::u32string &NAME_PARTS::stem() const
{
    return stem_;
}

const std::u32string &NAME_PARTS::ext() const
{
    return ext_;
}

bool NAME_PARTS::has_ext() const
{
    return has_ext_;
}

NAME_PARTS::NAME_PARTS(std::u32string stem, std::u32string ext, bool has_ext, bool is_dir)
    : stem_(std::move(stem)),
      ext_(std::move(ext)),
      has_ext_(has_ext),
      is_dir_(is_dir)
{
}

std::u32string NAME_PARTS::whole() const
{
    return has_ext_ ? stem_ + U'.' + ext_ : stem_;
}
