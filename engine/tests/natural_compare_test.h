#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "natural_compare.h"
#include "platform.h"
#include "text_service.h"

class NATURAL_COMPARE_TEST : public testing::Test
{
protected:
    std::vector<std::u32string> sorted(std::vector<std::u32string> names, bool use_natural) const
    {
        std::sort(names.begin(), names.end(),
            [this, use_natural](const std::u32string &a, const std::u32string &b) {
                return compare_.compare(a, b, use_natural) < 0;
            });
        return names;
    }

    std::unique_ptr<TEXT_SERVICE> text_service_ = PLATFORM::create_text_service();
    NATURAL_COMPARE compare_{*text_service_};
};
