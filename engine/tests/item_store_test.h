#pragma once

#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "add_result.h"
#include "fake_fs_env.h"
#include "item.h"
#include "item_store.h"
#include "platform.h"
#include "real_fs_env.h"

#if defined(_WIN32)
#include <windows.h>
#endif

class ITEM_STORE_TEST : public testing::Test
{
protected:
    std::filesystem::path at(const std::filesystem::path &relative) const
    {
        return env_.root() / relative;
    }

    std::vector<std::u32string> names() const
    {
        std::vector<std::u32string> result;
        for (const uint64_t id : store_.order()) {
            result.push_back(store_.find(id)->name);
        }
        return result;
    }

    std::vector<uint64_t> add_files(const std::vector<std::u32string> &file_names)
    {
        std::vector<std::filesystem::path> paths;
        for (const std::u32string &name : file_names) {
            const std::filesystem::path path = env_.root() / UTF::path_from_u32(name);
            env_.make_file(path, 1);
            paths.push_back(path);
        }
        store_.add(paths, ADD_MODE::as_items);
        return store_.order();
    }

    static SORT_STATE state(SORT_KEY key, bool descending)
    {
        SORT_STATE value;
        value.key = key;
        value.descending = descending;
        return value;
    }

    static ADD_RESULT result(uint32_t added, uint32_t skipped, uint32_t failed)
    {
        ADD_RESULT value;
        value.added = added;
        value.skipped = skipped;
        value.failed = failed;
        return value;
    }

    FAKE_FS_ENV env_;
    ITEM_STORE store_{env_.file_system(), env_.text_service()};
};
