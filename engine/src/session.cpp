#include "session.h"

SESSION::SESSION(std::unique_ptr<TEXT_SERVICE> text_service,
    std::unique_ptr<FILE_SYSTEM> file_system, bool windows_host)
    : text_service_(required(std::move(text_service))),
      file_system_(required(std::move(file_system))),
      checker_(windows_host),
      items_(*file_system_, *text_service_),
      mods_(*text_service_),
      cache_(*file_system_, *text_service_),
      builder_(*text_service_, checker_),
      finder_(builder_, checker_),
      planner_(*text_service_),
      executor_(*file_system_, *text_service_)
{
}

const char *SESSION::version()
{
    constexpr const char PREFIX[] = "LIMENAMER_VERSION=";
    return VERSION_TOKEN + sizeof(PREFIX) - 1;
}

const TEXT_SERVICE &SESSION::text_service() const
{
    return *text_service_;
}

FILE_SYSTEM &SESSION::file_system()
{
    return *file_system_;
}

ADD_RESULT SESSION::add_items(const std::vector<std::filesystem::path> &paths, ADD_MODE mode)
{
    const ADD_RESULT result = recorded([&]() {
        return items_.add(paths, mode);
    });
    track_folders();
    return result;
}

size_t SESSION::remove_items(const std::vector<uint64_t> &ids)
{
    const size_t removed = recorded([&]() {
        return items_.remove(ids);
    });
    track_folders();
    return removed;
}

void SESSION::clear_items()
{
    recorded([&]() {
        items_.clear();
    });
    track_folders();
}

std::optional<bool> SESSION::move_items(const std::vector<uint64_t> &ids, uint64_t target_id,
    bool after, bool dry_run)
{
    ensure_idle();
    if (dry_run) {
        return items_.move(ids, target_id, after, true);
    }
    return recorded([&]() {
        return items_.move(ids, target_id, after, false);
    });
}

void SESSION::sort_items(SORT_KEY key, bool descending)
{
    recorded([&]() {
        items_.sort(key, descending);
    });
}

const SORT_STATE &SESSION::sort_state() const
{
    return items_.sort_state();
}

void SESSION::refresh_folders()
{
    ensure_idle();
    cache_.refresh();
}

const ITEM_STORE &SESSION::items() const
{
    return items_;
}

const std::vector<MOD_ENTRY> &SESSION::mods() const
{
    return mods_.entries();
}

uint64_t SESSION::add_mod(MODIFICATION mod)
{
    return recorded([&]() {
        return mods_.add(std::move(mod));
    });
}

bool SESSION::update_mod(uint64_t id, MODIFICATION mod)
{
    return recorded([&]() {
        return mods_.update(id, std::move(mod));
    });
}

bool SESSION::remove_mod(uint64_t id)
{
    return recorded([&]() {
        return mods_.remove(id);
    });
}

std::optional<bool> SESSION::move_mod(uint64_t id, uint64_t target_id, bool after,
    bool dry_run)
{
    ensure_idle();
    if (dry_run) {
        return mods_.move(id, target_id, after, true);
    }
    return recorded([&]() {
        return mods_.move(id, target_id, after, false);
    });
}

bool SESSION::set_mod_enabled(uint64_t id, bool enabled)
{
    return recorded([&]() {
        return mods_.set_enabled(id, enabled);
    });
}

void SESSION::set_all_mods_enabled(bool enabled)
{
    recorded([&]() {
        mods_.set_all_enabled(enabled);
    });
}

std::optional<uint64_t> SESSION::direct_edit(uint64_t item_id, std::u32string new_name)
{
    ensure_idle();
    if (!items_.contains(item_id)) {
        return std::nullopt;
    }
    return recorded([&]() {
        return mods_.direct_edit(item_id, std::move(new_name));
    });
}

std::optional<EXAMPLE> SESSION::example(const MODIFICATION &draft, uint64_t before_mod_id) const
{
    return finder_.find(items_, mods_.entries(), draft, before_mod_id);
}

const OPTIONS &SESSION::options() const
{
    return options_;
}

void SESSION::set_options(OPTIONS options)
{
    options.duplicate.custom_text = text_service_->to_nfc(options.duplicate.custom_text);
    recorded([&]() {
        options_ = std::move(options);
    });
}

PREVIEW SESSION::preview() const
{
    return builder_.build(items_, mods_.entries(), options_, cache_);
}

bool SESSION::undo()
{
    ensure_idle();
    std::optional<SNAPSHOT> previous = history_.undo(snapshot());
    if (!previous) {
        return false;
    }
    restore(std::move(*previous));
    return true;
}

bool SESSION::redo()
{
    ensure_idle();
    std::optional<SNAPSHOT> next = history_.redo(snapshot());
    if (!next) {
        return false;
    }
    restore(std::move(*next));
    return true;
}

bool SESSION::can_undo() const
{
    return history_.can_undo();
}

bool SESSION::can_redo() const
{
    return history_.can_redo();
}

std::optional<RENAME_REPORT> SESSION::rename(const RENAME_EXECUTOR::PROGRESS &progress)
{
    ensure_idle();
    cache_.refresh();
    const PREVIEW current = preview();
    if (current.error > 0 || current.conflict > 0) {
        return std::nullopt;
    }
    std::vector<RENAME_REQUEST> requests;
    for (const PREVIEW_ROW &row : current.rows) {
        if (!row.changed) {
            continue;
        }
        const ITEM &item = *items_.find(row.item_id);
        RENAME_REQUEST request;
        request.item_id = item.id;
        request.path = item.path;
        request.name = item.name;
        request.new_name = row.new_name;
        request.is_dir = item.info.is_dir;
        requests.push_back(std::move(request));
    }
    if (requests.empty()) {
        return RENAME_REPORT();
    }
    const RENAME_REPORT report =
        executor_.run(requests, planner_.plan(requests, cache_), progress);
    if (!executor_.pending()) {
        finish_rename();
    }
    return report;
}

RENAME_REPORT SESSION::resolve_rename(RESOLVE_CHOICE choice,
    const RENAME_EXECUTOR::PROGRESS &progress)
{
    if (!executor_.pending()) {
        throw std::logic_error("no rename is waiting for a choice");
    }
    const RENAME_REPORT report = executor_.resolve(choice, progress);
    finish_rename();
    return report;
}

bool SESSION::rename_pending() const
{
    return executor_.pending();
}

const RENAME_RECORD &SESSION::record() const
{
    return record_;
}

RENAME_REPORT SESSION::undo_check() const
{
    const UNDO_CHECK check = record_.check(*file_system_, *text_service_);
    RENAME_REPORT report;
    report.total = static_cast<uint32_t>(record_.size());
    report.skipped = static_cast<uint32_t>(check.skipped.size());
    report.problems = check.skipped;
    return report;
}

std::optional<RENAME_REPORT> SESSION::undo_rename(const RENAME_EXECUTOR::PROGRESS &progress)
{
    ensure_idle();
    if (record_.empty()) {
        return std::nullopt;
    }
    const UNDO_CHECK check = record_.check(*file_system_, *text_service_);
    if (check.requests.empty()) {
        return std::nullopt;
    }
    FOLDER_CACHE folders(*file_system_, *text_service_);
    for (const RENAME_REQUEST &request : check.requests) {
        folders.load(request.path.parent_path());
    }
    RENAME_REPORT report = executor_.run(check.requests,
        planner_.plan(check.requests, folders), progress, false);
    report.total = static_cast<uint32_t>(record_.size());
    report.skipped = static_cast<uint32_t>(check.skipped.size());
    report.problems.insert(report.problems.begin(), check.skipped.begin(), check.skipped.end());
    finish_undo();
    return report;
}

SNAPSHOT SESSION::snapshot() const
{
    SNAPSHOT current;
    current.order = items_.order();
    current.sort = items_.sort_state();
    current.mods = mods_.entries();
    current.options = options_;
    return current;
}

void SESSION::restore(SNAPSHOT snapshot)
{
    items_.restore(snapshot.order, snapshot.sort);
    mods_.restore(std::move(snapshot.mods));
    options_ = std::move(snapshot.options);
    track_folders();
}

void SESSION::track_folders()
{
    std::vector<std::filesystem::path> folders;
    std::unordered_set<std::filesystem::path::string_type> seen;
    for (const uint64_t id : items_.order()) {
        const std::filesystem::path &folder = items_.find(id)->folder;
        if (seen.insert(folder.native()).second) {
            folders.push_back(folder);
        }
    }
    cache_.retain(folders);
    for (const std::filesystem::path &folder : folders) {
        cache_.load(folder);
    }
}

void SESSION::ensure_idle() const
{
    if (executor_.pending()) {
        throw SESSION_BUSY();
    }
}

void SESSION::finish_rename()
{
    std::unordered_map<uint64_t, std::filesystem::path> targets;
    FOLDER_MOVES folders;
    std::vector<RECORD_ENTRY> entries;
    for (const RENAME_TRACK &track : executor_.tracks()) {
        if (track.failed || track.reverted || track.current == track.original) {
            continue;
        }
        targets.emplace(track.item_id, track.current);
        if (track.is_dir) {
            folders.add(track.original, track.current);
        }
        RECORD_ENTRY entry;
        entry.item_id = track.item_id;
        entry.before = track.original;
        entry.after = track.current;
        file_system_->get_info(track.current, entry.info);
        entries.push_back(std::move(entry));
    }
    if (entries.empty()) {
        return;
    }

    history_.clear();
    items_.prune();
    items_.relocate(targets, folders);
    record_.set(std::move(entries));
    mods_.set_all_enabled(false);
    track_folders();
    cache_.refresh();
}

void SESSION::finish_undo()
{
    std::unordered_map<uint64_t, std::filesystem::path> targets;
    std::unordered_set<uint64_t> restored;
    FOLDER_MOVES folders;
    for (const RENAME_TRACK &track : executor_.tracks()) {
        if (track.failed || track.current == track.original) {
            continue;
        }
        restored.insert(track.item_id);
        if (track.is_dir) {
            folders.add(track.original, track.current);
        }
        const std::optional<uint64_t> listed = items_.listed_at(track.original);
        if (listed) {
            targets.emplace(*listed, track.current);
        }
    }
    if (restored.empty()) {
        return;
    }

    history_.clear();
    items_.prune();
    items_.relocate(targets, folders);
    std::vector<RECORD_ENTRY> remaining;
    for (const RECORD_ENTRY &entry : record_.entries()) {
        if (restored.contains(entry.item_id)) {
            continue;
        }
        RECORD_ENTRY kept = entry;
        const std::optional<std::filesystem::path> moved = folders.follow(kept.after);
        if (moved) {
            kept.after = *moved;
        }
        remaining.push_back(std::move(kept));
    }
    record_.set(std::move(remaining));
    track_folders();
    cache_.refresh();
}
