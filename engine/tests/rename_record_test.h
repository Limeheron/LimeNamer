#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "fake_fs_env.h"
#include "file_info.h"
#include "record_entry.h"
#include "rename_record.h"
#include "undo_check.h"

class RENAME_RECORD_TEST : public testing::Test
{
protected:
    RECORD_ENTRY renamed(uint64_t id, const std::filesystem::path &before,
        const std::filesystem::path &after, bool is_dir = false)
    {
        const std::filesystem::path path = env_.root() / after;
        if (is_dir) {
            env_.make_folder(path);
        } else {
            env_.make_file(path, 3);
        }
        RECORD_ENTRY entry;
        entry.item_id = id;
        entry.before = env_.root() / before;
        entry.after = path;
        env_.file_system().get_info(path, entry.info);
        return entry;
    }

    UNDO_CHECK check(std::vector<RECORD_ENTRY> entries)
    {
        record_.set(std::move(entries));
        return record_.check(env_.file_system(), env_.text_service());
    }

    static std::vector<RENAME_FAILURE> reasons(const UNDO_CHECK &result)
    {
        std::vector<RENAME_FAILURE> values;
        for (const REPORT_ITEM &item : result.skipped) {
            values.push_back(item.failure);
        }
        return values;
    }

    FAKE_FS_ENV env_;
    RENAME_RECORD record_;
};
