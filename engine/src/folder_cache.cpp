#include "folder_cache.h"

FOLDER_CACHE::FOLDER_CACHE(const FILE_SYSTEM &file_system, const TEXT_SERVICE &text_service)
    : file_system_(file_system),
      text_service_(text_service)
{
}

void FOLDER_CACHE::load(const std::filesystem::path &folder)
{
    const std::optional<std::u32string> key = key_of(folder);
    if (key && !folders_.contains(*key)) {
        folders_.emplace(*key, read(folder));
    }
}

void FOLDER_CACHE::reload(const std::filesystem::path &folder)
{
    const std::optional<std::u32string> key = key_of(folder);
    if (key) {
        folders_.insert_or_assign(*key, read(folder));
    }
}

void FOLDER_CACHE::refresh()
{
    for (auto &[key, names] : folders_) {
        names = read(names.folder);
    }
}

void FOLDER_CACHE::retain(const std::vector<std::filesystem::path> &folders)
{
    std::unordered_set<std::u32string> keep;
    for (const std::filesystem::path &folder : folders) {
        const std::optional<std::u32string> key = key_of(folder);
        if (key) {
            keep.insert(*key);
        }
    }
    std::erase_if(folders_, [&keep](const auto &entry) {
        return !keep.contains(entry.first);
    });
}

bool FOLDER_CACHE::has_name(const std::filesystem::path &folder, std::u32string_view name) const
{
    const FOLDER_NAMES *names = find(folder);
    return names != nullptr && names->keys.contains(text_service_.fold_key(name));
}

const FOLDER_NAMES *FOLDER_CACHE::find(const std::filesystem::path &folder) const
{
    const std::optional<std::u32string> key = key_of(folder);
    if (!key) {
        return nullptr;
    }
    return find_key(*key);
}

const FOLDER_NAMES *FOLDER_CACHE::find_key(const std::u32string &folder_key) const
{
    const auto names = folders_.find(folder_key);
    return names == folders_.end() ? nullptr : &names->second;
}

size_t FOLDER_CACHE::size() const
{
    return folders_.size();
}

std::optional<std::u32string> FOLDER_CACHE::key_of(const std::filesystem::path &folder) const
{
    const std::optional<std::u32string> text = UTF::path_to_u32(folder.lexically_normal());
    if (!text) {
        return std::nullopt;
    }
    return text_service_.fold_key(*text);
}

FOLDER_NAMES FOLDER_CACHE::read(const std::filesystem::path &folder) const
{
    FOLDER_NAMES names;
    names.folder = folder;
    std::vector<std::filesystem::path> entries;
    if (!file_system_.list_names(folder, entries).ok()) {
        return names;
    }
    names.readable = true;
    for (const std::filesystem::path &entry : entries) {
        const std::optional<std::u32string> text = UTF::path_to_u32(entry);
        if (text) {
            names.keys.insert(text_service_.fold_key(*text));
        }
    }
    return names;
}
