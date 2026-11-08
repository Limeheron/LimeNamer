#pragma once

#include <memory>
#include <new>
#include <stdexcept>

#include <gtest/gtest.h>

#include "c_guard.h"
#include "fake_file_system.h"
#include "limenamer.h"
#include "ln_session.h"
#include "mod_json_error.h"
#include "platform.h"
#include "session.h"
#include "session_busy.h"
#include "text_service.h"

class C_GUARD_TEST : public testing::Test
{
protected:
    std::unique_ptr<TEXT_SERVICE> text_ = PLATFORM::create_text_service();
    LN_SESSION session_{PLATFORM::create_text_service(),
        std::make_unique<FAKE_FILE_SYSTEM>(*text_), true};
};
