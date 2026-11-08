#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "mod_applier.h"
#include "mod_checker.h"
#include "mod_json.h"
#include "modification.h"
#include "platform.h"
#include "text_service.h"
#include "token_context.h"
#include "token_formatter.h"
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

    static std::u32string text(const std::string &value)
    {
        return UTF::decode_utf8(value).value();
    }

    std::unique_ptr<TEXT_SERVICE> text_service_ = PLATFORM::create_text_service();
    MOD_APPLIER applier_{*text_service_};
};
