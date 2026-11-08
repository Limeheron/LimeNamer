#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "fake_fs_env.h"
#include "folder_cache.h"
#include "fs_error_output.h"
#include "rename_planner.h"
#include "rename_request.h"
#include "rename_step.h"
#include "utf.h"

class RENAME_PLANNER_TEST : public testing::Test
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
        return value;
    }

    std::vector<RENAME_STEP> plan(const std::vector<RENAME_REQUEST> &requests)
    {
        for (const RENAME_REQUEST &value : requests) {
            cache_.load(value.path.parent_path());
        }
        return planner_.plan(requests, cache_);
    }

    void execute(const std::vector<RENAME_STEP> &steps)
    {
        for (const RENAME_STEP &step : steps) {
            ASSERT_EQ(env_.file_system().rename(step.from, step.to).error, FS_ERROR::none)
                << step.from << " -> " << step.to;
        }
    }

    std::vector<std::filesystem::path> names(const std::filesystem::path &relative = "")
    {
        std::vector<std::filesystem::path> result;
        env_.file_system().list_names(env_.root() / relative, result);
        std::sort(result.begin(), result.end());
        return result;
    }

    static std::vector<STEP_KIND> kinds(const std::vector<RENAME_STEP> &steps)
    {
        std::vector<STEP_KIND> result;
        for (const RENAME_STEP &step : steps) {
            result.push_back(step.kind);
        }
        return result;
    }

    FAKE_FS_ENV env_;
    FOLDER_CACHE cache_{env_.file_system(), env_.text_service()};
    RENAME_PLANNER planner_{env_.text_service()};
};
