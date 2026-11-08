#include "index_list.h"

INDEX_LIST_RESULT INDEX_LIST::parse(std::u32string_view text, uint32_t item_count)
{
    INDEX_LIST_RESULT result;
    std::vector<std::pair<uint64_t, uint64_t>> ranges;
    size_t at = 0;
    const auto skip_spaces = [&]() {
        while (at < text.size() && is_space(text[at])) {
            ++at;
        }
    };
    const auto read_number = [&]() -> std::optional<uint64_t> {
        skip_spaces();
        if (at >= text.size() || !is_digit(text[at])) {
            return std::nullopt;
        }
        uint64_t value = 0;
        while (at < text.size() && is_digit(text[at])) {
            value = std::min<uint64_t>(value * 10 + (text[at] - U'0'), UINT32_MAX + uint64_t{1});
            ++at;
        }
        skip_spaces();
        return value;
    };

    skip_spaces();
    if (at == text.size()) {
        return result;
    }
    while (true) {
        const std::optional<uint64_t> first = read_number();
        if (!first) {
            result.error = INDEX_ERROR::syntax;
            return result;
        }
        uint64_t last = *first;
        if (at < text.size() && is_range_mark(text[at])) {
            ++at;
            const std::optional<uint64_t> second = read_number();
            if (!second) {
                result.error = INDEX_ERROR::syntax;
                return result;
            }
            last = *second;
        }
        ranges.emplace_back(std::min(*first, last), std::max(*first, last));
        if (at == text.size()) {
            break;
        }
        if (text[at] != U',') {
            result.error = INDEX_ERROR::syntax;
            return result;
        }
        ++at;
    }

    for (const auto &[low, high] : ranges) {
        if (low == 0 || high > item_count) {
            result.error = INDEX_ERROR::out_of_range;
            return result;
        }
    }
    for (const auto &[low, high] : ranges) {
        for (uint64_t index = low; index <= high; ++index) {
            result.indices.push_back(static_cast<uint32_t>(index));
        }
    }
    std::sort(result.indices.begin(), result.indices.end());
    result.indices.erase(std::unique(result.indices.begin(), result.indices.end()),
        result.indices.end());
    return result;
}

std::u32string INDEX_LIST::format(std::vector<uint32_t> indices)
{
    std::sort(indices.begin(), indices.end());
    indices.erase(std::unique(indices.begin(), indices.end()), indices.end());
    std::u32string text;
    size_t at = 0;
    while (at < indices.size()) {
        size_t end = at + 1;
        while (end < indices.size() && indices[end] == indices[end - 1] + 1) {
            ++end;
        }
        if (!text.empty()) {
            text += U", ";
        }
        if (end - at >= MIN_RANGE) {
            text += number_text(indices[at]) + U"-" + number_text(indices[end - 1]);
            at = end;
        } else {
            text += number_text(indices[at]);
            ++at;
        }
    }
    return text;
}

bool INDEX_LIST::is_space(char32_t c)
{
    return c == U' ' || c == U'\t' || c == U'　';
}

bool INDEX_LIST::is_digit(char32_t c)
{
    return c >= U'0' && c <= U'9';
}

bool INDEX_LIST::is_range_mark(char32_t c)
{
    return c == U'-' || c == U'~';
}

std::u32string INDEX_LIST::number_text(uint32_t value)
{
    const std::string digits = std::to_string(value);
    std::u32string text;
    for (const char c : digits) {
        text.push_back(static_cast<char32_t>(c));
    }
    return text;
}
