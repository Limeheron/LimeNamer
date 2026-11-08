#pragma once

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <memory>
#include <regex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "fake_file_system.h"
#include "file_system.h"
#include "mod_json.h"
#include "platform.h"
#include "preview.h"
#include "session.h"
#include "text_service.h"
#include "utf.h"

class SESSION_TEST : public testing::Test
{
protected:
    SESSION_TEST()
    {
        std::unique_ptr<FAKE_FILE_SYSTEM> file_system =
            std::make_unique<FAKE_FILE_SYSTEM>(*fake_text_);
        fake_ = file_system.get();
        session_ = std::make_unique<SESSION>(PLATFORM::create_text_service(),
            std::move(file_system), true);
        fake_->add_folder(root_);
    }

    std::vector<std::filesystem::path> files(const std::vector<std::u32string> &names)
    {
        std::vector<std::filesystem::path> paths;
        for (const std::u32string &name : names) {
            const std::filesystem::path path = root_ / UTF::path_from_u32(name);
            fake_->add_file(path, 1);
            paths.push_back(path);
        }
        return paths;
    }

    std::vector<std::u32string> new_names() const
    {
        std::vector<std::u32string> names;
        for (const PREVIEW_ROW &row : session_->preview().rows) {
            names.push_back(row.new_name);
        }
        return names;
    }

    static MODIFICATION mod(const char *json)
    {
        return MOD_JSON::parse(json);
    }

#if defined(_WIN32)
    const std::filesystem::path root_ = L"C:\\session";
#else
    const std::filesystem::path root_ = "/session";
#endif
    std::unique_ptr<TEXT_SERVICE> fake_text_ = PLATFORM::create_text_service();
    FAKE_FILE_SYSTEM *fake_ = nullptr;
    std::unique_ptr<SESSION> session_;
};
