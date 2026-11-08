#include "conflict_resolver.h"

CONFLICT_RESOLVER::CONFLICT_RESOLVER(const TEXT_SERVICE &text_service)
    : text_service_(text_service)
{
}

std::vector<CONFLICT> CONFLICT_RESOLVER::find(const std::vector<CONFLICT_ENTRY> &entries,
    const FOLDER_CACHE &cache) const
{
    const size_t count = entries.size();
    std::vector<std::u32string> folder_keys(count);
    std::vector<std::u32string> current_keys(count);
    std::vector<std::u32string> final_keys(count);
    std::unordered_map<std::filesystem::path::string_type, std::u32string> folder_memo;
    std::unordered_map<std::u32string, std::unordered_set<std::u32string>> listed;
    std::unordered_map<std::u32string, std::unordered_map<std::u32string, std::vector<size_t>>>
        finals;

    for (size_t index = 0; index < count; ++index) {
        const CONFLICT_ENTRY &entry = entries[index];
        const auto memo = folder_memo.find(entry.folder.native());
        if (memo != folder_memo.end()) {
            folder_keys[index] = memo->second;
        } else {
            folder_keys[index] = cache.key_of(entry.folder).value_or(std::u32string());
            folder_memo.emplace(entry.folder.native(), folder_keys[index]);
        }
        current_keys[index] = text_service_.fold_key(entry.current_name);
        final_keys[index] =
            entry.changed ? text_service_.fold_key(entry.new_name) : current_keys[index];
        listed[folder_keys[index]].insert(current_keys[index]);
        finals[folder_keys[index]][final_keys[index]].push_back(index);
    }

    std::vector<CONFLICT> conflicts(count);
    for (size_t index = 0; index < count; ++index) {
        if (!entries[index].changed) {
            continue;
        }
        const std::vector<size_t> &same = finals[folder_keys[index]][final_keys[index]];
        if (same.size() > 1) {
            const size_t other = same[0] == index ? same[1] : same[0];
            conflicts[index] = {CONFLICT_KIND::list_item, entries[other].id};
            continue;
        }
        if (listed[folder_keys[index]].contains(final_keys[index])) {
            continue;
        }
        const FOLDER_NAMES *names = cache.find_key(folder_keys[index]);
        if (names != nullptr && names->keys.contains(final_keys[index])) {
            conflicts[index] = {CONFLICT_KIND::existing_item, 0};
        }
    }
    return conflicts;
}
