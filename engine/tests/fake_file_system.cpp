#include "fake_file_system.h"

FAKE_FILE_SYSTEM::FAKE_FILE_SYSTEM(const TEXT_SERVICE &text_service)
    : text_service_(text_service)
{
}

void FAKE_FILE_SYSTEM::add_folder(const std::filesystem::path &path)
{
    add_entry(path, true, 0, false);
}

void FAKE_FILE_SYSTEM::add_file(const std::filesystem::path &path, int64_t size, bool hidden)
{
    add_entry(path, false, size, hidden);
}

void FAKE_FILE_SYSTEM::fail_next_rename(const std::filesystem::path &from, FS_ERROR error)
{
    rename_failures_[key_of(from)].push_back(error);
}

FS_STATUS FAKE_FILE_SYSTEM::get_info(const std::filesystem::path &path, FILE_INFO &info) const
{
    const auto entry = entries_.find(key_of(path));
    if (entry == entries_.end()) {
        return {FS_ERROR::not_found, 0};
    }
    info = entry->second.info;
    return {};
}

FS_STATUS FAKE_FILE_SYSTEM::list_names(const std::filesystem::path &folder,
    std::vector<std::filesystem::path> &names) const
{
    const std::u32string folder_key = key_of(folder);
    const auto folder_entry = entries_.find(folder_key);
    if (folder_entry == entries_.end() || !folder_entry->second.info.is_dir) {
        return {FS_ERROR::not_found, 0};
    }
    std::vector<std::filesystem::path> result;
    for (const auto &[key, entry] : entries_) {
        if (entry.parent_key == folder_key && key != folder_key) {
            result.push_back(entry.path.filename());
        }
    }
    names = std::move(result);
    return {};
}

FS_STATUS FAKE_FILE_SYSTEM::rename(const std::filesystem::path &from,
    const std::filesystem::path &to)
{
    const std::u32string from_key = key_of(from);
    const auto failure = rename_failures_.find(from_key);
    if (failure != rename_failures_.end()) {
        const FS_STATUS status {failure->second.front(), 0};
        failure->second.pop_front();
        if (failure->second.empty()) {
            rename_failures_.erase(failure);
        }
        return status;
    }
    const auto source = entries_.find(from_key);
    if (source == entries_.end()) {
        return {FS_ERROR::not_found, 0};
    }
    const auto parent = entries_.find(key_of(to.parent_path()));
    if (parent == entries_.end() || !parent->second.info.is_dir) {
        return {FS_ERROR::not_found, 0};
    }
    const std::u32string to_key = key_of(to);
    const std::u32string prefix = from_key + SEPARATOR;
    if (entries_.contains(to_key)) {
        return {FS_ERROR::already_exists, 0};
    }
    if (to_key.starts_with(prefix)) {
        return {FS_ERROR::other, 0};
    }

    const std::filesystem::path source_path = source->second.path;
    const std::filesystem::path target_path = parent->second.path / to.filename();
    std::vector<std::u32string> old_keys;
    std::vector<std::pair<std::u32string, FAKE_ENTRY>> moved;
    for (const auto &[key, entry] : entries_) {
        if (key != from_key && !key.starts_with(prefix)) {
            continue;
        }
        FAKE_ENTRY next = entry;
        const std::filesystem::path relative = entry.path.lexically_relative(source_path);
        next.path = relative == "." ? target_path : target_path / relative;
        next.parent_key = key_of(next.path.parent_path());
        old_keys.push_back(key);
        moved.emplace_back(key_of(next.path), std::move(next));
    }

    for (const std::u32string &key : old_keys) {
        entries_.erase(key);
    }
    for (auto &[key, entry] : moved) {
        entries_.emplace(key, std::move(entry));
    }
    return {};
}

std::u32string FAKE_FILE_SYSTEM::key_of(const std::filesystem::path &path) const
{
    std::filesystem::path normal = path.lexically_normal();
    normal.make_preferred();
    std::optional<std::u32string> text = UTF::path_to_u32(normal);
    if (!text) {
        throw std::invalid_argument("invalid fake path");
    }
    while (text->size() > 1 && text->back() == SEPARATOR) {
        text->pop_back();
    }
    return text_service_.fold_key(*text);
}

void FAKE_FILE_SYSTEM::add_entry(const std::filesystem::path &path, bool is_dir, int64_t size,
    bool hidden)
{
    const std::u32string key = key_of(path);
    if (entries_.contains(key)) {
        throw std::logic_error("fake entry already exists");
    }
    FAKE_ENTRY entry;
    entry.path = path;
    entry.parent_key = key_of(path.parent_path());
    if (!entries_.empty()) {
        const auto parent = entries_.find(entry.parent_key);
        if (parent == entries_.end() || !parent->second.info.is_dir) {
            throw std::logic_error("fake entry has no parent folder");
        }
        entry.path = parent->second.path / path.filename();
    }
    entry.info.is_dir = is_dir;
    entry.info.hidden = hidden;
    entry.info.size = is_dir ? 0 : size;
    entry.info.id = {true, 1, 0, next_id_++};
    entries_.emplace(key, std::move(entry));
}

void FAKE_FILE_SYSTEM::set_modified(const std::filesystem::path &path, int64_t modified_ms)
{
    entries_.at(key_of(path)).info.modified_ms = modified_ms;
}
