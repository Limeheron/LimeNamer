#include "mod_applier.h"

MOD_APPLIER::MOD_APPLIER(const TEXT_SERVICE &text_service)
    : text_service_(text_service),
      finder_(text_service)
{
}

std::u32string MOD_APPLIER::apply(const MODIFICATION &mod, std::u32string_view name,
    bool is_dir, std::u32string_view value) const
{
    const NAME_PARTS parts = NAME_PARTS::split(name, is_dir);
    if (!parts.applies(mod.part)) {
        return std::u32string(name);
    }
    const std::u32string text = parts.text_of(mod.part);
    const std::vector<SPAN> spans = finder_.find(mod.position, text);
    if (spans.empty()) {
        return std::u32string(name);
    }
    const std::u32string edited = edit(text, spans, mod.kind, value);
    if (edited == text) {
        return std::u32string(name);
    }
    return parts.join(mod.part, text_service_.to_nfc(edited));
}

std::u32string MOD_APPLIER::edit(std::u32string_view text, const std::vector<SPAN> &spans,
    MOD_KIND kind, std::u32string_view value)
{
    std::u32string result(text);
    for (auto span = spans.rbegin(); span != spans.rend(); ++span) {
        const size_t length = span->end - span->begin;
        switch (kind) {
        case MOD_KIND::insert:
            result.insert(span->begin, value);
            break;
        case MOD_KIND::replace:
            result.replace(span->begin, length, value);
            break;
        case MOD_KIND::remove:
            result.erase(span->begin, length);
            break;
        case MOD_KIND::format:
            break;
        }
    }
    return result;
}
