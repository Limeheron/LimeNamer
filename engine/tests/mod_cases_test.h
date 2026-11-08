#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "fake_fs_env.h"
#include "folder_cache.h"
#include "item_store.h"
#include "mod_checker.h"
#include "mod_json.h"
#include "mod_store.h"
#include "name_checker.h"
#include "options.h"
#include "preview.h"
#include "preview_builder.h"
#include "utf.h"

class MOD_CASES_TEST : public testing::Test
{
protected:
    static nlohmann::json load_cases()
    {
        const std::filesystem::path path =
            std::filesystem::path(LN_TESTDATA_DIR) / "mod_cases.json";
        std::ifstream in(path, std::ios::binary);
        if (!in) {
            throw std::runtime_error("cannot open test data: " + path.string());
        }
        std::ostringstream content;
        content << in.rdbuf();
        return nlohmann::json::parse(content.str());
    }

    static std::filesystem::path path_of(const std::string &text)
    {
        return UTF::path_from_utf8(text).value();
    }

#if defined(_WIN32)
    static constexpr const bool WINDOWS_HOST = true;
#else
    static constexpr const bool WINDOWS_HOST = false;
#endif
};
