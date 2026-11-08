#include "span_finder.h"

SPAN_FINDER::SPAN_FINDER(const TEXT_SERVICE &text_service)
    : text_service_(text_service)
{
    for (size_t c = 0; c < ASCII_COUNT; ++c) {
        ascii_folds_[c] = text_service_.fold_key(std::u32string(1, static_cast<char32_t>(c)));
    }
}

std::vector<SPAN> SPAN_FINDER::find(const POSITION &position, std::u32string_view text) const
{
    if (is_insert(position.type)) {
        return find_points(position, text);
    }
    return find_ranges(position, text);
}

std::vector<SPAN> SPAN_FINDER::matches(std::u32string_view text, std::u32string_view needle,
    bool case_sensitive) const
{
    std::vector<SPAN> found;
    if (needle.empty() || needle.size() > text.size()) {
        return found;
    }
    if (case_sensitive) {
        size_t from = 0;
        while (true) {
            const size_t at = text.find(needle, from);
            if (at == std::u32string_view::npos) {
                break;
            }
            found.push_back({at, at + needle.size()});
            from = at + needle.size();
        }
        return found;
    }
    const std::vector<std::u32string> folded_text = fold_each(text);
    const std::vector<std::u32string> folded_needle = fold_each(needle);
    size_t at = 0;
    while (at + needle.size() <= text.size()) {
        const bool equal = std::equal(folded_needle.begin(), folded_needle.end(),
            folded_text.begin() + static_cast<std::ptrdiff_t>(at));
        if (equal) {
            found.push_back({at, at + needle.size()});
            at += needle.size();
        } else {
            ++at;
        }
    }
    return found;
}

bool SPAN_FINDER::is_insert(POSITION_TYPE type)
{
    switch (type) {
    case POSITION_TYPE::start:
    case POSITION_TYPE::end:
    case POSITION_TYPE::after_nth:
    case POSITION_TYPE::before_nth_from_end:
    case POSITION_TYPE::before_text:
    case POSITION_TYPE::after_text:
        return true;
    default:
        return false;
    }
}

std::vector<SPAN> SPAN_FINDER::select(const std::vector<SPAN> &found, OCCURRENCE occurrence,
    uint32_t n)
{
    if (found.empty()) {
        return {};
    }
    switch (occurrence) {
    case OCCURRENCE::first:
        return {found.front()};
    case OCCURRENCE::last:
        return {found.back()};
    case OCCURRENCE::nth:
        if (n == 0 || n > found.size()) {
            return {};
        }
        return {found[n - 1]};
    case OCCURRENCE::nth_from_end:
        if (n == 0 || n > found.size()) {
            return {};
        }
        return {found[found.size() - n]};
    case OCCURRENCE::all:
        return found;
    }
    return {};
}

std::vector<SPAN> SPAN_FINDER::find_points(const POSITION &position,
    std::u32string_view text) const
{
    const size_t length = text.size();
    switch (position.type) {
    case POSITION_TYPE::start:
        return {{0, 0}};
    case POSITION_TYPE::end:
        return {{length, length}};
    case POSITION_TYPE::after_nth: {
        const size_t at = std::min<size_t>(position.n, length);
        return {{at, at}};
    }
    case POSITION_TYPE::before_nth_from_end: {
        const size_t at = length - std::min<size_t>(position.n, length);
        return {{at, at}};
    }
    case POSITION_TYPE::before_text:
    case POSITION_TYPE::after_text: {
        const bool before = position.type == POSITION_TYPE::before_text;
        std::vector<SPAN> points;
        for (const SPAN &match : select(matches(text, position.text, position.case_sensitive),
                 position.occurrence, position.occurrence_n)) {
            const size_t at = before ? match.begin : match.end;
            points.push_back({at, at});
        }
        return points;
    }
    default:
        throw std::logic_error("not an insert position");
    }
}

std::vector<SPAN> SPAN_FINDER::find_ranges(const POSITION &position,
    std::u32string_view text) const
{
    static_cast<void>(position);
    static_cast<void>(text);
    throw std::logic_error("range positions are not supported yet");
}

std::vector<std::u32string> SPAN_FINDER::fold_each(std::u32string_view text) const
{
    std::vector<std::u32string> folded;
    folded.reserve(text.size());
    for (const char32_t c : text) {
        if (c < ASCII_COUNT) {
            folded.push_back(ascii_folds_[c]);
        } else {
            folded.push_back(text_service_.fold_key(std::u32string(1, c)));
        }
    }
    return folded;
}
