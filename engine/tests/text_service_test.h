#pragma once

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "platform.h"
#include "text_service.h"

class TEXT_SERVICE_TEST : public testing::Test
{
protected:
    std::unique_ptr<TEXT_SERVICE> service_ = PLATFORM::create_text_service();
};
