#include "rename_executor.h"

RENAME_EXECUTOR::RENAME_EXECUTOR(FILE_SYSTEM &file_system, const TEXT_SERVICE &text_service)
    : file_system_(file_system),
      text_service_(text_service),
      planner_(text_service)
{
}

RENAME_REPORT RENAME_EXECUTOR::run(const std::vector<RENAME_REQUEST> &requests,
    const std::vector<RENAME_STEP> &steps, const PROGRESS &progress, bool allow_choice)
{
    tracks_.clear();
    index_.clear();
    problems_.clear();
    pending_ = false;
    for (const RENAME_REQUEST &request : requests) {
        RENAME_TRACK track;
        track.item_id = request.item_id;
        track.original = request.path;
        track.target = request.path.parent_path() / UTF::path_from_u32(request.new_name);
        track.current = request.path;
        track.is_dir = request.is_dir;
        index_.emplace(track.item_id, tracks_.size());
        tracks_.push_back(std::move(track));
    }

    const FAILURES failures = execute(steps, progress);
    std::unordered_map<uint64_t, REPORT_ITEM> restore_problems;
    for (RENAME_TRACK &item : tracks_) {
        const auto failure = failures.find(item.item_id);
        if (failure == failures.end()) {
            continue;
        }
        item.failed = true;
        item.failure = failure->second.first;
        item.os_error = failure->second.second;
        if (item.current.filename() != item.original.filename()) {
            const FS_STATUS status =
                move(item, item.current.parent_path() / item.original.filename());
            if (!status.ok()) {
                restore_problems.emplace(item.item_id, REPORT_ITEM{item.item_id, item.original,
                    item.target, item.current, REPORT_STAGE::revert, failure_of(status.error),
                    status.os_error});
            }
        }
    }

    bool any_succeeded = false;
    for (const RENAME_TRACK &item : tracks_) {
        if (!item.failed) {
            any_succeeded = true;
            continue;
        }
        problems_.push_back({item.item_id, item.original, item.target, item.current,
            REPORT_STAGE::run, item.failure, item.os_error});
        const auto restore = restore_problems.find(item.item_id);
        if (restore != restore_problems.end()) {
            problems_.push_back(restore->second);
        }
    }
    pending_ = allow_choice && !problems_.empty() && any_succeeded;
    return report();
}

RENAME_REPORT RENAME_EXECUTOR::resolve(RESOLVE_CHOICE choice, const PROGRESS &progress)
{
    if (!pending_) {
        throw std::logic_error("no rename is waiting for a choice");
    }
    pending_ = false;
    if (choice == RESOLVE_CHOICE::keep_succeeded) {
        return report();
    }

    std::vector<RENAME_REQUEST> requests;
    std::unordered_map<uint64_t, std::filesystem::path> before;
    FOLDER_CACHE cache(file_system_, text_service_);
    for (const RENAME_TRACK &item : tracks_) {
        if (item.failed || item.current == item.original) {
            continue;
        }
        RENAME_REQUEST request;
        request.item_id = item.item_id;
        request.path = item.current;
        request.name = text_service_.to_nfc(UTF::path_to_u32(item.current.filename()).value());
        request.new_name = UTF::path_to_u32(item.original.filename()).value();
        request.is_dir = item.is_dir;
        requests.push_back(std::move(request));
        before.emplace(item.item_id, item.current);
        cache.load(item.current.parent_path());
    }

    const FAILURES failures = execute(planner_.plan(requests, cache), progress);
    for (const RENAME_REQUEST &request : requests) {
        RENAME_TRACK &item = track(request.item_id);
        const auto failure = failures.find(request.item_id);
        if (failure == failures.end()) {
            item.reverted = true;
            continue;
        }
        const std::filesystem::path &previous = before.at(request.item_id);
        if (item.current.filename() != previous.filename()) {
            move(item, item.current.parent_path() / previous.filename());
        }
        problems_.push_back({item.item_id, item.original, item.target, item.current,
            REPORT_STAGE::revert, failure->second.first, failure->second.second});
    }
    restore_stuck();
    return report();
}

bool RENAME_EXECUTOR::pending() const
{
    return pending_;
}

const std::vector<RENAME_TRACK> &RENAME_EXECUTOR::tracks() const
{
    return tracks_;
}

RENAME_FAILURE RENAME_EXECUTOR::failure_of(FS_ERROR error)
{
    switch (error) {
    case FS_ERROR::none:
        return RENAME_FAILURE::none;
    case FS_ERROR::not_found:
        return RENAME_FAILURE::not_found;
    case FS_ERROR::already_exists:
        return RENAME_FAILURE::already_exists;
    case FS_ERROR::access_denied:
        return RENAME_FAILURE::access_denied;
    case FS_ERROR::in_use:
        return RENAME_FAILURE::in_use;
    case FS_ERROR::path_too_long:
        return RENAME_FAILURE::path_too_long;
    case FS_ERROR::other:
        return RENAME_FAILURE::other;
    }
    return RENAME_FAILURE::other;
}

RENAME_EXECUTOR::FAILURES RENAME_EXECUTOR::execute(const std::vector<RENAME_STEP> &steps,
    const PROGRESS &progress)
{
    FAILURES failures;
    std::unordered_set<std::u32string> held;
    const uint32_t total = static_cast<uint32_t>(steps.size());
    uint32_t done = 0;
    for (const RENAME_STEP &step : steps) {
        RENAME_TRACK &item = track(step.item_id);
        if (!failures.contains(item.item_id)) {
            if (!held.empty() && held.contains(key_of(step.to))) {
                failures.emplace(item.item_id,
                    std::make_pair(RENAME_FAILURE::depends_on_failed, int32_t{0}));
                held.insert(key_of(item.current));
            } else {
                const FS_STATUS status = move(item, step.to);
                if (!status.ok()) {
                    failures.emplace(item.item_id,
                        std::make_pair(failure_of(status.error), status.os_error));
                    held.insert(key_of(item.current));
                } else if (item.is_dir && !held.empty()) {
                    held = held_keys(failures);
                }
            }
        }
        ++done;
        if (progress) {
            progress(done, total);
        }
    }
    return failures;
}

std::unordered_set<std::u32string> RENAME_EXECUTOR::held_keys(const FAILURES &failures) const
{
    std::unordered_set<std::u32string> keys;
    for (const auto &[id, failure] : failures) {
        keys.insert(key_of(tracks_[index_.at(id)].current));
    }
    return keys;
}

void RENAME_EXECUTOR::restore_stuck()
{
    for (RENAME_TRACK &item : tracks_) {
        if (!item.failed || item.current.filename() == item.original.filename()) {
            continue;
        }
        if (move(item, item.current.parent_path() / item.original.filename()).ok()) {
            std::erase_if(problems_, [&item](const REPORT_ITEM &problem) {
                return problem.item_id == item.item_id && problem.stage == REPORT_STAGE::revert;
            });
        }
    }
}

FS_STATUS RENAME_EXECUTOR::move(RENAME_TRACK &item, const std::filesystem::path &to)
{
    const FS_STATUS status = file_system_.rename(item.current, to);
    if (!status.ok()) {
        return status;
    }
    const std::filesystem::path from = item.current;
    item.current = to;
    if (item.is_dir) {
        for (RENAME_TRACK &other : tracks_) {
            if (&other != &item && PATHS::is_under(other.current, from)) {
                other.current = PATHS::moved(other.current, from, to);
            }
        }
    }
    return status;
}

RENAME_TRACK &RENAME_EXECUTOR::track(uint64_t id)
{
    return tracks_[index_.at(id)];
}

std::u32string RENAME_EXECUTOR::key_of(const std::filesystem::path &path) const
{
    return text_service_.fold_key(UTF::path_to_u32(path).value_or(std::u32string()));
}

RENAME_REPORT RENAME_EXECUTOR::report() const
{
    RENAME_REPORT result;
    result.problems = problems_;
    result.total = static_cast<uint32_t>(tracks_.size());
    for (const RENAME_TRACK &item : tracks_) {
        if (item.failed) {
            ++result.failed;
        } else if (!item.reverted) {
            ++result.succeeded;
        }
    }
    result.needs_choice = pending_;
    return result;
}
