#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "mod_entry.h"
#include "mod_json.h"
#include "mod_store.h"
#include "platform.h"
#include "text_service.h"

class MOD_STORE_TEST : public testing::Test
{
protected:
    uint64_t add(const char *json)
    {
        return store_.add(MOD_JSON::parse(json));
    }

    std::vector<uint64_t> ids() const
    {
        std::vector<uint64_t> result;
        for (const MOD_ENTRY &entry : store_.entries()) {
            result.push_back(entry.id);
        }
        return result;
    }

    static constexpr const char *INSERT_A =
        R"({"kind":"insert","position":{"type":"start"},"value":[{"text":"a"}]})";
    static constexpr const char *INSERT_B =
        R"({"kind":"insert","position":{"type":"end"},"value":[{"text":"b"}]})";
    static constexpr const char *DELETE_ALL =
        R"({"kind":"delete","part":"ext","position":{"type":"all"}})";

    std::unique_ptr<TEXT_SERVICE> text_service_ = PLATFORM::create_text_service();
    MOD_STORE store_{*text_service_};
};
