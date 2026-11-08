#include "c_input.h"

static_assert(static_cast<int32_t>(SORT_KEY::added) == ln_sort_key_added);
static_assert(static_cast<int32_t>(SORT_KEY::name_natural) == ln_sort_key_name_natural);
static_assert(static_cast<int32_t>(SORT_KEY::name_ordinal) == ln_sort_key_name_ordinal);
static_assert(static_cast<int32_t>(SORT_KEY::modified) == ln_sort_key_modified);
static_assert(static_cast<int32_t>(SORT_KEY::custom) == ln_sort_key_custom);

static_assert(static_cast<int32_t>(ADD_MODE::as_items) == ln_add_mode_as_items);
static_assert(static_cast<int32_t>(ADD_MODE::folder_contents) == ln_add_mode_folder_contents);

static_assert(static_cast<int32_t>(RESOLVE_CHOICE::revert_all) == ln_resolve_choice_revert_all);
static_assert(static_cast<int32_t>(RESOLVE_CHOICE::keep_succeeded)
    == ln_resolve_choice_keep_succeeded);

std::optional<std::u32string> C_INPUT::text(const char *value)
{
    if (value == nullptr) {
        return std::nullopt;
    }
    return UTF::decode_utf8(value);
}

std::optional<std::filesystem::path> C_INPUT::path(const char *value)
{
    if (value == nullptr) {
        return std::nullopt;
    }
    return UTF::path_from_utf8(value);
}

std::optional<std::vector<std::filesystem::path>> C_INPUT::paths(const char *const *values,
    size_t count)
{
    if (values == nullptr && count > 0) {
        return std::nullopt;
    }
    std::vector<std::filesystem::path> result;
    result.reserve(count);
    for (size_t index = 0; index < count; ++index) {
        std::optional<std::filesystem::path> value = path(values[index]);
        if (!value) {
            return std::nullopt;
        }
        result.push_back(std::move(*value));
    }
    return result;
}

std::optional<std::vector<uint64_t>> C_INPUT::ids(const uint64_t *values, size_t count)
{
    if (values == nullptr && count > 0) {
        return std::nullopt;
    }
    if (count == 0) {
        return std::vector<uint64_t>();
    }
    return std::vector<uint64_t>(values, values + count);
}

std::optional<bool> C_INPUT::flag(uint8_t value)
{
    if (value > 1) {
        return std::nullopt;
    }
    return value == 1;
}
