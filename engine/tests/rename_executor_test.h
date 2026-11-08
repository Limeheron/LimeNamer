#pragma once

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "fake_fs_env.h"
#include "folder_cache.h"
#include "fs_error_output.h"
#include "rename_executor.h"
#include "rename_planner.h"
#include "rename_report.h"
#include "rename_request.h"
#include "utf.h"

class RENAME_EXECUTOR_TEST : public testing::Test
{
protected:
    RENAME_REQUEST request(uint64_t id, const std::filesystem::path &relative,
        std::u32string new_name, bool is_dir = false)
    {
        const std::filesystem::path path = env_.root() / relative;
        if (is_dir) {
            env_.make_folder(path);
        } else {
            env_.make_file(path, 1);
        }
        RENAME_REQUEST value;
        value.item_id = id;
        value.path = path;
        value.name = UTF::path_to_u32(path.filename()).value();
        value.new_name = std::move(new_name);
        value.is_dir = is_dir;
        return value;
    }

    RENAME_REPORT run(const std::vector<RENAME_REQUEST> &requests)
    {
        FOLDER_CACHE cache(env_.file_system(), env_.text_service());
        for (const RENAME_REQUEST &value : requests) {
            cache.load(value.path.parent_path());
        }
        return executor_.run(requests, planner_.plan(requests, cache), {});
    }

    void fail(const std::filesystem::path &relative, FS_ERROR error)
    {
        env_.fake().fail_next_rename(env_.root() / relative, error);
    }

    std::vector<std::filesystem::path> names(const std::filesystem::path &relative = "")
    {
        std::vector<std::filesystem::path> result;
        env_.file_system().list_names(env_.root() / relative, result);
        std::sort(result.begin(), result.end());
        return result;
    }

    const RENAME_TRACK &track(uint64_t id) const
    {
        for (const RENAME_TRACK &item : executor_.tracks()) {
            if (item.item_id == id) {
                return item;
            }
        }
        throw std::out_of_range("no track");
    }

    FAKE_FS_ENV env_;
    RENAME_PLANNER planner_{env_.text_service()};
    RENAME_EXECUTOR executor_{env_.file_system(), env_.text_service()};
};
