#include "natural_compare.h"

NATURAL_COMPARE::NATURAL_COMPARE(const TEXT_SERVICE &text_service)
    : text_service_(text_service)
{
}

std::u32string NATURAL_COMPARE::key(std::u32string_view name) const
{
    std::u32string folded(name);
    bool ascii = true;
    for (char32_t &c : folded) {
        if (c >= 0x80u) {
            ascii = false;
            break;
        }
        if (c >= U'A' && c <= U'Z') {
            c = c - U'A' + U'a';
        }
    }
    if (ascii) {
        return folded;
    }
    return text_service_.to_lower(name);
}

int NATURAL_COMPARE::compare(std::u32string_view a, std::u32string_view b, bool use_natural) const
{
    const int result = compare_keys(key(a), key(b), use_natural);
    if (result != 0) {
        return result;
    }
    return compare_ordinal(a, b);
}

int NATURAL_COMPARE::compare_keys(std::u32string_view a, std::u32string_view b, bool use_natural)
{
    return use_natural ? compare_natural(a, b) : compare_ordinal(a, b);
}

bool NATURAL_COMPARE::is_digit(char32_t c)
{
    return c >= U'0' && c <= U'9';
}

int NATURAL_COMPARE::compare_ordinal(std::u32string_view a, std::u32string_view b)
{
    const int result = a.compare(b);
    return result < 0 ? -1 : (result > 0 ? 1 : 0);
}

int NATURAL_COMPARE::compare_natural(std::u32string_view a, std::u32string_view b)
{
    size_t i = 0;
    size_t j = 0;
    int zeros = 0;
    while (i < a.size() && j < b.size()) {
        if (!is_digit(a[i]) || !is_digit(b[j])) {
            if (a[i] != b[j]) {
                return a[i] < b[j] ? -1 : 1;
            }
            ++i;
            ++j;
            continue;
        }
        size_t a_end = i;
        while (a_end < a.size() && is_digit(a[a_end])) {
            ++a_end;
        }
        size_t b_end = j;
        while (b_end < b.size() && is_digit(b[b_end])) {
            ++b_end;
        }
        size_t a_start = i;
        while (a_start + 1 < a_end && a[a_start] == U'0') {
            ++a_start;
        }
        size_t b_start = j;
        while (b_start + 1 < b_end && b[b_start] == U'0') {
            ++b_start;
        }
        const size_t a_length = a_end - a_start;
        const size_t b_length = b_end - b_start;
        if (a_length != b_length) {
            return a_length < b_length ? -1 : 1;
        }
        const int digits = a.substr(a_start, a_length).compare(b.substr(b_start, b_length));
        if (digits != 0) {
            return digits < 0 ? -1 : 1;
        }
        if (zeros == 0 && a_start - i != b_start - j) {
            zeros = a_start - i < b_start - j ? -1 : 1;
        }
        i = a_end;
        j = b_end;
    }
    if (i < a.size() || j < b.size()) {
        return i < a.size() ? 1 : -1;
    }
    return zeros;
}
