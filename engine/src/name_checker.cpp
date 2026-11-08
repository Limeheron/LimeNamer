#include "name_checker.h"

NAME_CHECKER::NAME_CHECKER(bool windows_host)
    : windows_host_(windows_host)
{
}

NAME_CHECK NAME_CHECKER::check(std::u32string_view name, std::u32string_view current_name,
    size_t folder_units) const
{
    const uint32_t found = issues(name, folder_units);
    const uint32_t before = issues(current_name, folder_units);
    const uint32_t error_mask =
        NAME_ISSUE::COMMON | (windows_host_ ? NAME_ISSUE::WINDOWS : NAME_ISSUE::MAC);
    const uint32_t warning_mask =
        NAME_ISSUE::HIDDEN | (windows_host_ ? NAME_ISSUE::MAC : NAME_ISSUE::WINDOWS);
    NAME_CHECK result;
    result.errors = found & error_mask;
    if (UTF::utf16_length(name) <= UTF::utf16_length(current_name)) {
        result.errors &= ~NAME_ISSUE::WIN_PATH_TOO_LONG;
    }
    result.warnings = found & warning_mask & ~before;
    return result;
}

uint32_t NAME_CHECKER::issues(std::u32string_view name, size_t folder_units) const
{
    uint32_t found = 0;
    if (is_blank(name)) {
        found |= NAME_ISSUE::EMPTY;
    }
    const bool dot_name = name == U"." || name == U"..";
    if (dot_name) {
        found |= NAME_ISSUE::DOT_NAME;
    }
    for (const char32_t c : name) {
        if (c < 0x20u || c == U'\\' || c == U'/' || c == U':' || c == U'*' || c == U'?'
            || c == U'"' || c == U'<' || c == U'>' || c == U'|') {
            found |= NAME_ISSUE::WIN_FORBIDDEN_CHAR;
        }
        if (c == U'/' || c == U':' || c == U'\0') {
            found |= NAME_ISSUE::MAC_FORBIDDEN_CHAR;
        }
    }
    if (is_reserved(name)) {
        found |= NAME_ISSUE::WIN_RESERVED;
    }
    if (!name.empty() && (name.back() == U' ' || name.back() == U'.')) {
        found |= NAME_ISSUE::WIN_TRAILING;
    }
    const size_t utf16 = UTF::utf16_length(name);
    if (utf16 > MAX_NAME_UNITS) {
        found |= NAME_ISSUE::WIN_NAME_TOO_LONG;
    }
    if (windows_host_ && folder_units + utf16 > MAX_PATH_UNITS) {
        found |= NAME_ISSUE::WIN_PATH_TOO_LONG;
    }
    if (UTF::utf8_length(name) > MAX_NAME_UNITS) {
        found |= NAME_ISSUE::MAC_NAME_TOO_LONG;
    }
    if (!dot_name && !name.empty() && name.front() == U'.') {
        found |= NAME_ISSUE::HIDDEN;
    }
    return found;
}

size_t NAME_CHECKER::folder_units(std::u32string_view folder)
{
    const size_t units = UTF::utf16_length(folder);
    if (!folder.empty() && (folder.back() == U'\\' || folder.back() == U'/')) {
        return units;
    }
    return units + 1;
}

bool NAME_CHECKER::is_blank(std::u32string_view name)
{
    for (const char32_t c : name) {
        if (c != U' ' && c != U'\t' && c != U' ' && c != U'　') {
            return false;
        }
    }
    return true;
}

bool NAME_CHECKER::is_reserved(std::u32string_view name)
{
    std::u32string base(name.substr(0, name.find(U'.')));
    while (!base.empty() && base.back() == U' ') {
        base.pop_back();
    }
    for (char32_t &c : base) {
        if (c >= U'a' && c <= U'z') {
            c = c - U'a' + U'A';
        }
    }
    if (base == U"CON" || base == U"PRN" || base == U"AUX" || base == U"NUL") {
        return true;
    }
    if (base.size() != 4 || (base.substr(0, 3) != U"COM" && base.substr(0, 3) != U"LPT")) {
        return false;
    }
    const char32_t last = base[3];
    return (last >= U'0' && last <= U'9') || last == U'¹' || last == U'²'
        || last == U'³';
}
