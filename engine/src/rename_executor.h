#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "file_system.h"
#include "folder_cache.h"
#include "paths.h"
#include "rename_failure.h"
#include "rename_planner.h"
#include "rename_report.h"
#include "rename_request.h"
#include "rename_step.h"
#include "rename_track.h"
#include "report_item.h"
#include "resolve_choice.h"
#include "text_service.h"
#include "utf.h"

class RENAME_EXECUTOR
{
public:
    using PROGRESS = std::function<void(uint32_t done, uint32_t total)>;
    using FAILURES = std::unordered_map<uint64_t, std::pair<RENAME_FAILURE, int32_t>>;

    RENAME_EXECUTOR(FILE_SYSTEM &file_system, const TEXT_SERVICE &text_service);

    RENAME_REPORT run(const std::vector<RENAME_REQUEST> &requests,
        const std::vector<RENAME_STEP> &steps, const PROGRESS &progress,
        bool allow_choice = true);
    RENAME_REPORT resolve(RESOLVE_CHOICE choice, const PROGRESS &progress);

    bool pending() const;
    const std::vector<RENAME_TRACK> &tracks() const;

    static RENAME_FAILURE failure_of(FS_ERROR error);

private:
    FAILURES execute(const std::vector<RENAME_STEP> &steps, const PROGRESS &progress);
    std::unordered_set<std::u32string> held_keys(const FAILURES &failures) const;
    void restore_stuck();
    FS_STATUS move(RENAME_TRACK &track, const std::filesystem::path &to);
    RENAME_TRACK &track(uint64_t id);
    std::u32string key_of(const std::filesystem::path &path) const;
    RENAME_REPORT report() const;

    FILE_SYSTEM &file_system_;
    const TEXT_SERVICE &text_service_;
    RENAME_PLANNER planner_;
    std::vector<RENAME_TRACK> tracks_;
    std::unordered_map<uint64_t, size_t> index_;
    std::vector<REPORT_ITEM> problems_;
    bool pending_ = false;
};
