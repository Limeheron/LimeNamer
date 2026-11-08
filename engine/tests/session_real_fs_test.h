#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "add_mode.h"
#include "file_info.h"
#include "file_lock.h"
#include "fs_error_output.h"
#include "mod_json.h"
#include "platform.h"
#include "real_fs_env.h"
#include "rename_failure.h"
#include "rename_report.h"
#include "resolve_choice.h"
#include "session.h"
#include "utf.h"

class SESSION_REAL_FS_TEST : public testing::Test
{
protected:
    std::filesystem::path at(const std::filesystem::path &relative) const
    {
        return env_.root() / relative;
    }

    std::vector<std::filesystem::path> files(const std::vector<std::u32string> &names)
    {
        std::vector<std::filesystem::path> paths;
        int64_t size = 1;
        for (const std::u32string &name : names) {
            const std::filesystem::path path = env_.root() / UTF::path_from_u32(name);
            env_.make_file(path, size++);
            paths.push_back(path);
        }
        return paths;
    }

    std::vector<std::filesystem::path> names_in(const std::filesystem::path &folder)
    {
        std::vector<std::filesystem::path> names;
        EXPECT_TRUE(env_.file_system().list_names(folder, names).ok());
        std::sort(names.begin(), names.end());
        return names;
    }

    std::vector<std::u32string> item_names() const
    {
        std::vector<std::u32string> names;
        for (const uint64_t id : session_.items().order()) {
            names.push_back(session_.items().find(id)->name);
        }
        return names;
    }

    int64_t size_of(const std::filesystem::path &path)
    {
        FILE_INFO info;
        EXPECT_TRUE(env_.file_system().get_info(path, info).ok());
        return info.size;
    }

    static std::vector<std::filesystem::path> sorted(std::vector<std::filesystem::path> names)
    {
        std::sort(names.begin(), names.end());
        return names;
    }

    static std::filesystem::path extended(const std::filesystem::path &path)
    {
#if defined(_WIN32)
        return std::filesystem::path(L"\\\\?\\" + path.native());
#else
        return path;
#endif
    }

    static MODIFICATION mod(const char *json)
    {
        return MOD_JSON::parse(json);
    }

    REAL_FS_ENV env_;
    SESSION session_{PLATFORM::create_text_service(), PLATFORM::create_file_system(),
        PLATFORM::WINDOWS_HOST};
};
