#include "conflict_resolver.h"

CONFLICT_RESOLVER::CONFLICT_RESOLVER(const TEXT_SERVICE &text_service)
    : text_service_(text_service)
{
}

std::vector<CONFLICT> CONFLICT_RESOLVER::find(const std::vector<CONFLICT_ENTRY> &entries,
    const FOLDER_CACHE &cache) const
{
    const size_t count = entries.size();
    const CONFLICT_KEYS keys = prepare(entries, cache);
    std::unordered_map<std::u32string, std::unordered_set<std::u32string>> listed;
    std::unordered_map<std::u32string, std::unordered_map<std::u32string, std::vector<size_t>>>
        finals;
    for (size_t index = 0; index < count; ++index) {
        listed[keys.folder[index]].insert(keys.current[index]);
        finals[keys.folder[index]][keys.final[index]].push_back(index);
    }

    std::vector<CONFLICT> conflicts(count);
    for (size_t index = 0; index < count; ++index) {
        if (!entries[index].changed) {
            continue;
        }
        const std::vector<size_t> &same = finals[keys.folder[index]][keys.final[index]];
        if (same.size() > 1) {
            const size_t other = same[0] == index ? same[1] : same[0];
            conflicts[index] = {CONFLICT_KIND::list_item, entries[other].id};
            continue;
        }
        if (listed[keys.folder[index]].contains(keys.final[index])) {
            continue;
        }
        const FOLDER_NAMES *names = cache.find_key(keys.folder[index]);
        if (names != nullptr && names->keys.contains(keys.final[index])) {
            conflicts[index] = {CONFLICT_KIND::existing_item, 0};
        }
    }
    return conflicts;
}

std::vector<std::optional<std::u32string>> CONFLICT_RESOLVER::adjust(
    const std::vector<CONFLICT_ENTRY> &entries, const FOLDER_CACHE &cache,
    const DUPLICATE_OPTIONS &options) const
{
    const size_t count = entries.size();
    std::vector<std::optional<std::u32string>> adjusted(count);
    if (!options.enabled) {
        return adjusted;
    }
    const CONFLICT_KEYS keys = prepare(entries, cache);

    std::unordered_map<std::u32string, std::unordered_set<std::u32string>> occupied;
    for (size_t index = 0; index < count; ++index) {
        if (!occupied.contains(keys.folder[index])) {
            const FOLDER_NAMES *names = cache.find_key(keys.folder[index]);
            occupied[keys.folder[index]] =
                names != nullptr ? names->keys : std::unordered_set<std::u32string>();
        }
    }
    for (size_t index = 0; index < count; ++index) {
        if (entries[index].changed) {
            occupied[keys.folder[index]].erase(keys.current[index]);
        }
    }
    std::unordered_map<std::u32string, std::unordered_set<std::u32string>> wanted;
    std::unordered_map<std::u32string, std::unordered_map<std::u32string, size_t>> counts;
    for (size_t index = 0; index < count; ++index) {
        if (entries[index].changed) {
            wanted[keys.folder[index]].insert(keys.final[index]);
            ++counts[keys.folder[index]][keys.final[index]];
        } else {
            occupied[keys.folder[index]].insert(keys.current[index]);
        }
    }

    const auto baseline = occupied;
    std::unordered_map<std::u32string, std::unordered_set<std::u32string>> claimed;
    std::unordered_map<std::u32string, std::unordered_map<std::u32string, uint64_t>> next;
    for (size_t index = 0; index < count; ++index) {
        if (!entries[index].changed) {
            continue;
        }
        const std::u32string &folder = keys.folder[index];
        const std::u32string &key = keys.final[index];
        bool keep = false;
        if (!baseline.at(folder).contains(key)) {
            keep = options.mode == DUPLICATE_MODE::keep_first ? !claimed[folder].contains(key)
                                                              : counts[folder][key] == 1;
        }
        if (keep) {
            claimed[folder].insert(key);
            occupied[folder].insert(key);
            continue;
        }
        uint64_t &number = next[folder][key];
        if (number == 0) {
            number = 1;
        }
        while (true) {
            if (number > MAX_NUMBER) {
                throw std::length_error("too many duplicate names");
            }
            std::u32string candidate = numbered(entries[index].new_name, entries[index].is_dir,
                number, options);
            ++number;
            const std::u32string candidate_key = text_service_.fold_key(candidate);
            if (!occupied[folder].contains(candidate_key)
                && !wanted[folder].contains(candidate_key)) {
                occupied[folder].insert(candidate_key);
                adjusted[index] = std::move(candidate);
                break;
            }
        }
    }
    return adjusted;
}

std::u32string CONFLICT_RESOLVER::numbered(std::u32string_view name, bool is_dir,
    uint64_t number, const DUPLICATE_OPTIONS &options)
{
    const std::string digits = std::to_string(number);
    std::u32string text;
    for (const char c : digits) {
        text.push_back(static_cast<char32_t>(c));
    }
    std::u32string suffix;
    switch (options.format) {
    case DUPLICATE_FORMAT::paren:
        suffix = U" (" + text + U")";
        break;
    case DUPLICATE_FORMAT::underscore:
        suffix = U"_" + text;
        break;
    case DUPLICATE_FORMAT::dash:
        suffix = U"-" + text;
        break;
    case DUPLICATE_FORMAT::custom:
        suffix = options.custom_text + text;
        break;
    }
    const NAME_PARTS parts = NAME_PARTS::split(name, is_dir);
    return parts.join(MOD_PART::name, parts.stem() + suffix);
}

CONFLICT_KEYS CONFLICT_RESOLVER::prepare(const std::vector<CONFLICT_ENTRY> &entries,
    const FOLDER_CACHE &cache) const
{
    CONFLICT_KEYS keys;
    keys.folder.reserve(entries.size());
    keys.current.reserve(entries.size());
    keys.final.reserve(entries.size());
    std::unordered_map<std::filesystem::path::string_type, std::u32string> folders;
    for (const CONFLICT_ENTRY &entry : entries) {
        const auto known = folders.find(entry.folder.native());
        if (known != folders.end()) {
            keys.folder.push_back(known->second);
        } else {
            keys.folder.push_back(cache.key_of(entry.folder).value_or(std::u32string()));
            folders.emplace(entry.folder.native(), keys.folder.back());
        }
        keys.current.push_back(text_service_.fold_key(entry.current_name));
        keys.final.push_back(
            entry.changed ? text_service_.fold_key(entry.new_name) : keys.current.back());
    }
    return keys;
}
