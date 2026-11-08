#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>

#include "date_parts.h"
#include "modification.h"
#include "name_parts.h"
#include "number_settings.h"
#include "size_settings.h"
#include "token_context.h"

class TOKEN_FORMATTER
{
public:
    static std::u32string build(const MODIFICATION &mod, const TOKEN_CONTEXT &context);

    static std::u32string number(const NUMBER_SETTINGS &settings, uint64_t index);
    static std::u32string date(std::u32string_view format, int64_t ms);
    static std::u32string format_date(std::u32string_view format, const DATE_PARTS &parts);
    static std::optional<DATE_PARTS> local_parts(int64_t ms);
    static std::u32string size(const SIZE_SETTINGS &settings, int64_t bytes, bool is_dir);

private:
    static constexpr const uint64_t MAX_INDEX = 1000000000;

    static std::u32string digits(uint64_t value, size_t width);
};
