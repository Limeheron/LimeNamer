#pragma once

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <vector>

#include <gtest/gtest.h>

#include "fake_fs_env.h"
#include "file_info.h"
#include "file_system.h"
#include "folder_entry.h"
#include "fs_error_output.h"
#include "fs_status.h"
#include "real_fs_env.h"

template <typename ENV>
class FILE_SYSTEM_TEST : public testing::Test
{
protected:
    FILE_SYSTEM &fs()
    {
        return env_.file_system();
    }

    std::filesystem::path at(const std::filesystem::path &relative) const
    {
        return env_.root() / relative;
    }

    std::vector<std::filesystem::path> sorted_names(const std::filesystem::path &folder)
    {
        std::vector<std::filesystem::path> names;
        const FS_STATUS status = fs().list_names(folder, names);
        EXPECT_TRUE(status.ok());
        std::sort(names.begin(), names.end());
        return names;
    }

    int64_t size_of(const std::filesystem::path &path)
    {
        FILE_INFO info;
        EXPECT_TRUE(fs().get_info(path, info).ok());
        return info.size;
    }

    bool exists(const std::filesystem::path &path)
    {
        FILE_INFO info;
        return fs().get_info(path, info).ok();
    }

    ENV env_;
};
