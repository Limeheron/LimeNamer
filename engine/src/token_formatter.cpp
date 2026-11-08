#include "token_formatter.h"

std::u32string TOKEN_FORMATTER::build(const MODIFICATION &mod, const TOKEN_CONTEXT &context)
{
    std::u32string value;
    for (const VALUE_PIECE &piece : mod.value) {
        if (const std::u32string *text = std::get_if<std::u32string>(&piece)) {
            value += *text;
            continue;
        }
        switch (std::get<TOKEN_KIND>(piece)) {
        case TOKEN_KIND::number:
            value += number(mod.number, context.index);
            break;
        case TOKEN_KIND::original_name:
            value += NAME_PARTS::split(context.original_name, context.is_dir).stem();
            break;
        case TOKEN_KIND::modified_date:
            value += date(mod.modified_date.format, context.modified_ms);
            break;
        case TOKEN_KIND::created_date:
            value += date(mod.created_date.format, context.created_ms);
            break;
        case TOKEN_KIND::size:
            value += size(mod.size, context.size, context.is_dir);
            break;
        case TOKEN_KIND::parent_folder:
            value += context.parent_name;
            break;
        }
    }
    return value;
}

std::u32string TOKEN_FORMATTER::number(const NUMBER_SETTINGS &settings, uint64_t index)
{
    if (index > MAX_INDEX) {
        throw std::out_of_range("number token index is too large");
    }
    const int64_t value = settings.start + settings.step * static_cast<int64_t>(index);
    if (value < 0) {
        return U"-" + digits(static_cast<uint64_t>(-(value + 1)) + 1, settings.digits);
    }
    return digits(static_cast<uint64_t>(value), settings.digits);
}

std::u32string TOKEN_FORMATTER::date(std::u32string_view format, int64_t ms)
{
    const std::optional<DATE_PARTS> parts = local_parts(ms);
    if (!parts) {
        return {};
    }
    return format_date(format, *parts);
}

std::u32string TOKEN_FORMATTER::format_date(std::u32string_view format, const DATE_PARTS &parts)
{
    std::u32string result;
    size_t at = 0;
    while (at < format.size()) {
        const std::u32string_view rest = format.substr(at);
        if (rest.starts_with(U"yyyy")) {
            result += digits(static_cast<uint64_t>(parts.year), 4);
            at += 4;
        } else if (rest.starts_with(U"yy")) {
            result += digits(static_cast<uint64_t>(parts.year % 100), 2);
            at += 2;
        } else if (rest.starts_with(U"MM")) {
            result += digits(static_cast<uint64_t>(parts.month), 2);
            at += 2;
        } else if (rest.starts_with(U"dd")) {
            result += digits(static_cast<uint64_t>(parts.day), 2);
            at += 2;
        } else if (rest.starts_with(U"HH")) {
            result += digits(static_cast<uint64_t>(parts.hour), 2);
            at += 2;
        } else if (rest.starts_with(U"mm")) {
            result += digits(static_cast<uint64_t>(parts.minute), 2);
            at += 2;
        } else if (rest.starts_with(U"ss")) {
            result += digits(static_cast<uint64_t>(parts.second), 2);
            at += 2;
        } else {
            result.push_back(format[at]);
            ++at;
        }
    }
    return result;
}

std::optional<DATE_PARTS> TOKEN_FORMATTER::local_parts(int64_t ms)
{
    int64_t seconds = ms / 1000;
    if (ms % 1000 < 0) {
        --seconds;
    }
    const std::time_t time = static_cast<std::time_t>(seconds);
    std::tm local {};
#if defined(_WIN32)
    if (localtime_s(&local, &time) != 0) {
        return std::nullopt;
    }
#else
    if (localtime_r(&time, &local) == nullptr) {
        return std::nullopt;
    }
#endif
    DATE_PARTS parts;
    parts.year = local.tm_year + 1900;
    parts.month = local.tm_mon + 1;
    parts.day = local.tm_mday;
    parts.hour = local.tm_hour;
    parts.minute = local.tm_min;
    parts.second = local.tm_sec;
    return parts;
}

std::u32string TOKEN_FORMATTER::size(const SIZE_SETTINGS &settings, int64_t bytes, bool is_dir)
{
    if (is_dir || bytes < 0) {
        return {};
    }
    constexpr const std::array<std::u32string_view, 4> UNIT_TEXT = {U"B", U"KB", U"MB", U"GB"};
    const uint64_t value = static_cast<uint64_t>(bytes);
    size_t unit = 0;
    switch (settings.unit) {
    case SIZE_UNIT::bytes:
        unit = 0;
        break;
    case SIZE_UNIT::kb:
        unit = 1;
        break;
    case SIZE_UNIT::mb:
        unit = 2;
        break;
    case SIZE_UNIT::gb:
        unit = 3;
        break;
    case SIZE_UNIT::automatic:
        while (unit < 3 && value >= (uint64_t{1} << (10 * (unit + 1)))) {
            ++unit;
        }
        break;
    }
    uint64_t divisor = uint64_t{1} << (10 * unit);
    uint64_t rounded = (value + divisor / 2) / divisor;
    if (settings.unit == SIZE_UNIT::automatic && unit < 3 && rounded >= 1024) {
        ++unit;
        divisor <<= 10;
        rounded = (value + divisor / 2) / divisor;
    }
    return digits(rounded, 0) + std::u32string(UNIT_TEXT[unit]);
}

std::u32string TOKEN_FORMATTER::digits(uint64_t value, size_t width)
{
    const std::string text = std::to_string(value);
    std::u32string result;
    if (text.size() < width) {
        result.append(width - text.size(), U'0');
    }
    for (const char c : text) {
        result.push_back(static_cast<char32_t>(c));
    }
    return result;
}
