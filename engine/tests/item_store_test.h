#pragma once

#include <cstdint>
#include <filesystem>
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
