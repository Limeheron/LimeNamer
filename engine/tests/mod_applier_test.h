#pragma once

#include <memory>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "mod_applier.h"
#include "mod_json.h"
#include "modification.h"
#include "platform.h"
#include "text_service.h"

class MOD_APPLIER_TEST : public testing::Test
{
protected:
    std::u32string apply(const char *json, std::u32string_view name, std::u32string_view value,
        bool is_dir = false) const
    {
        return applier_.apply(MOD_JSON::parse(json), name, is_dir, value);
    }

    std::unique_ptr<TEXT_SERVICE> text_service_ = PLATFORM::create_text_service();
    MOD_APPLIER applier_{*text_service_};
};
