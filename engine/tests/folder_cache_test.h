#pragma once

#include <filesystem>
#include <vector>

#include <gtest/gtest.h>

#include "fake_fs_env.h"
#include "folder_cache.h"
#include "folder_names.h"

class FOLDER_CACHE_TEST : public testing::Test
{
protected:
    std::filesystem::path at(const std::filesystem::path &relative) const
    {
        return env_.root() / relative;
    }

    FAKE_FS_ENV env_;
    FOLDER_CACHE cache_{env_.file_system(), env_.text_service()};
};
