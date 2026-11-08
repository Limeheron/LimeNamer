#include "rename_record.h"

void RENAME_RECORD::set(std::vector<RECORD_ENTRY> entries)
{
    entries_ = std::move(entries);
}

void RENAME_RECORD::clear()
{
    entries_.clear();
}

bool RENAME_RECORD::empty() const
{
    return entries_.empty();
}

size_t RENAME_RECORD::size() const
{
    return entries_.size();
}

const std::vector<RECORD_ENTRY> &RENAME_RECORD::entries() const
{
    return entries_;
}

UNDO_CHECK RENAME_RECORD::check(const FILE_SYSTEM &file_system,
    const TEXT_SERVICE &text_service) const
{
    const size_t count = entries_.size();
    const auto key_of = [&text_service](const std::filesystem::path &path) {
        return text_service.fold_key(UTF::path_to_u32(path).value_or(std::u32string()));
    };
    std::vector<RENAME_FAILURE> reasons(count, RENAME_FAILURE::none);
    std::vector<std::filesystem::path> targets(count);
    std::vector<std::u32string> after_keys(count);
    std::vector<std::u32string> target_keys(count);
    std::vector<bool> is_dir(count, false);
    std::unordered_map<std::u32string, size_t> after_index;
    for (size_t index = 0; index < count; ++index) {
        const RECORD_ENTRY &entry = entries_[index];
        targets[index] = entry.after.parent_path() / entry.before.filename();
        after_keys[index] = key_of(entry.after);
        target_keys[index] = key_of(targets[index]);
        after_index.emplace(after_keys[index], index);
        FILE_INFO now;
        const FS_STATUS status = file_system.get_info(entry.after, now);
        if (!status.ok()) {
            reasons[index] = RENAME_EXECUTOR::failure_of(status.error);
        } else if (!same_file(entry.info, now)) {
            reasons[index] = RENAME_FAILURE::replaced;
        }
        is_dir[index] = now.is_dir;
    }

    std::vector<std::optional<size_t>> waits_for(count);
    for (size_t index = 0; index < count; ++index) {
        if (reasons[index] != RENAME_FAILURE::none || target_keys[index] == after_keys[index]) {
            continue;
        }
        FILE_INFO occupant;
        if (!file_system.get_info(targets[index], occupant).ok()) {
            continue;
        }
        const auto holder = after_index.find(target_keys[index]);
        if (holder != after_index.end() && holder->second != index) {
            waits_for[index] = holder->second;
        } else {
            reasons[index] = RENAME_FAILURE::already_exists;
        }
    }
    bool changed = true;
    while (changed) {
        changed = false;
        for (size_t index = 0; index < count; ++index) {
            if (reasons[index] == RENAME_FAILURE::none && waits_for[index]
                && reasons[*waits_for[index]] != RENAME_FAILURE::none) {
                reasons[index] = RENAME_FAILURE::depends_on_failed;
                changed = true;
            }
        }
    }

    UNDO_CHECK result;
    for (size_t index = 0; index < count; ++index) {
        const RECORD_ENTRY &entry = entries_[index];
        if (reasons[index] != RENAME_FAILURE::none) {
            result.skipped.push_back({entry.item_id, entry.after, entry.before, entry.after,
                REPORT_STAGE::check, reasons[index], 0});
            continue;
        }
        RENAME_REQUEST request;
        request.item_id = entry.item_id;
        request.path = entry.after;
        request.name = text_service.to_nfc(
            UTF::path_to_u32(entry.after.filename()).value_or(std::u32string()));
        request.new_name = text_service.to_nfc(
            UTF::path_to_u32(entry.before.filename()).value_or(std::u32string()));
        request.is_dir = is_dir[index];
        result.requests.push_back(std::move(request));
    }
    return result;
}

bool RENAME_RECORD::same_file(const FILE_INFO &recorded, const FILE_INFO &now)
{
    if (recorded.id.valid && now.id.valid) {
        return recorded.id == now.id;
    }
    if (recorded.is_dir != now.is_dir) {
        return false;
    }
    return recorded.is_dir
        || (recorded.size == now.size && recorded.modified_ms == now.modified_ms);
}
