#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <random>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

#include <gtest/gtest.h>

#include "limenamer.h"
#include "session.h"

class C_API_TEST : public testing::Test
{
protected:
    C_API_TEST()
    {
        std::random_device random;
        const std::filesystem::path base = std::filesystem::temp_directory_path();
        for (int attempt = 0; attempt < 16 && root_.empty(); ++attempt) {
            const std::filesystem::path candidate =
                base / ("limenamer_api_" + std::to_string(random()) + std::to_string(random()));
            if (std::filesystem::create_directory(candidate)) {
                root_ = candidate;
            }
        }
        if (root_.empty() || ln_session_create(&session_) != ln_result_ok) {
            throw std::runtime_error("failed to prepare api test");
        }
    }

    ~C_API_TEST() override
    {
        ln_session_destroy(session_);
        std::error_code error;
        std::filesystem::remove_all(root_, error);
    }

    static std::string utf8(const std::filesystem::path &path)
    {
        const std::u8string text = path.u8string();
        return std::string(text.begin(), text.end());
    }

    static std::filesystem::path from_utf8(const std::string &text)
    {
        return std::filesystem::path(std::u8string(text.begin(), text.end()));
    }

    std::string make_file(const std::string &name)
    {
        const std::filesystem::path path = root_ / from_utf8(name);
        std::ofstream out(path, std::ios::binary);
        out << "x";
        if (!out) {
            throw std::runtime_error("failed to create test file");
        }
        return utf8(path);
    }

    std::string make_folder(const std::string &name)
    {
        const std::filesystem::path path = root_ / from_utf8(name);
        std::filesystem::create_directory(path);
        return utf8(path);
    }

    LN_RESULT add(const std::vector<std::string> &paths, int32_t mode, LN_ADD_RESULT &result)
    {
        std::vector<const char *> values;
        for (const std::string &path : paths) {
            values.push_back(path.c_str());
        }
        return ln_items_add(session_, values.data(), values.size(), mode, &result);
    }

    std::vector<std::string> names()
    {
        std::vector<std::string> values;
        LN_PREVIEW *preview = nullptr;
        EXPECT_EQ(ln_preview(session_, &preview), ln_result_ok);
        if (preview != nullptr) {
            for (size_t index = 0; index < preview->count; ++index) {
                values.push_back(preview->rows[index].name);
            }
        }
        ln_preview_free(preview);
        return values;
    }

    std::vector<std::string> new_names()
    {
        std::vector<std::string> values;
        LN_PREVIEW *preview = nullptr;
        EXPECT_EQ(ln_preview(session_, &preview), ln_result_ok);
        if (preview != nullptr) {
            for (size_t index = 0; index < preview->count; ++index) {
                values.push_back(preview->rows[index].new_name);
            }
        }
        ln_preview_free(preview);
        return values;
    }

    std::string mods_json()
    {
        char *json = nullptr;
        EXPECT_EQ(ln_mods_get(session_, &json), ln_result_ok);
        const std::string text = json == nullptr ? std::string() : std::string(json);
        ln_string_free(json);
        return text;
    }

    std::vector<uint64_t> ids()
    {
        std::vector<uint64_t> values;
        LN_PREVIEW *preview = nullptr;
        EXPECT_EQ(ln_preview(session_, &preview), ln_result_ok);
        if (preview != nullptr) {
            for (size_t index = 0; index < preview->count; ++index) {
                values.push_back(preview->rows[index].item_id);
            }
        }
        ln_preview_free(preview);
        return values;
    }

    std::filesystem::path root_;
    LN_SESSION *session_ = nullptr;
};
