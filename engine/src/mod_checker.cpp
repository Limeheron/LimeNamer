#include "mod_checker.h"

std::vector<MOD_PROBLEM> MOD_CHECKER::check(const MODIFICATION &mod)
{
    std::vector<MOD_PROBLEM> problems;
    if (mod.scope == MOD_SCOPE::items && mod.targets.empty()) {
        problems.push_back({MOD_FIELD::targets, PROBLEM_REASON::required});
    }

    const POSITION &position = mod.position;
    switch (position.type) {
    case POSITION_TYPE::before_text:
    case POSITION_TYPE::after_text:
    case POSITION_TYPE::text:
        if (position.text.empty()) {
            problems.push_back({MOD_FIELD::find_text, PROBLEM_REASON::required});
        }
        break;
    case POSITION_TYPE::between:
        if (!position.from_beginning && position.start_text.empty()) {
            problems.push_back({MOD_FIELD::start_text, PROBLEM_REASON::required});
        }
        if (!position.to_end && position.end_text.empty()) {
            problems.push_back({MOD_FIELD::end_text, PROBLEM_REASON::required});
        }
        break;
    default:
        break;
    }

    const bool needs_value = mod.kind == MOD_KIND::insert || mod.kind == MOD_KIND::replace;
    if (needs_value && !has_value(mod.value)) {
        problems.push_back({MOD_FIELD::value, PROBLEM_REASON::required});
    }
    if (needs_value && uses(mod.value, TOKEN_KIND::modified_date)
        && mod.modified_date.format.empty()) {
        problems.push_back({MOD_FIELD::modified_date_format, PROBLEM_REASON::required});
    }
    if (needs_value && uses(mod.value, TOKEN_KIND::created_date)
        && mod.created_date.format.empty()) {
        problems.push_back({MOD_FIELD::created_date_format, PROBLEM_REASON::required});
    }
    return problems;
}

bool MOD_CHECKER::uses(const std::vector<VALUE_PIECE> &value, TOKEN_KIND token)
{
    for (const VALUE_PIECE &piece : value) {
        const TOKEN_KIND *kind = std::get_if<TOKEN_KIND>(&piece);
        if (kind != nullptr && *kind == token) {
            return true;
        }
    }
    return false;
}

bool MOD_CHECKER::has_value(const std::vector<VALUE_PIECE> &value)
{
    for (const VALUE_PIECE &piece : value) {
        const std::u32string *text = std::get_if<std::u32string>(&piece);
        if (text == nullptr || !text->empty()) {
            return true;
        }
    }
    return false;
}
