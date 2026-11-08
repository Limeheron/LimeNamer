#pragma once

#include <cstdint>
#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "platform.h"
#include "position.h"
#include "span.h"
#include "span_finder.h"
#include "text_service.h"

class SPAN_FINDER_TEST : public testing::Test
{
protected:
    static POSITION at(POSITION_TYPE type, uint32_t n = 0)
    {
        POSITION position;
        position.type = type;
        position.n = n;
        return position;
    }

    static POSITION around(POSITION_TYPE type, std::u32string_view text,
        OCCURRENCE occurrence = OCCURRENCE::first, uint32_t occurrence_n = 1,
        bool case_sensitive = false)
    {
        POSITION position;
        position.type = type;
        position.text = text;
        position.occurrence = occurrence;
        position.occurrence_n = occurrence_n;
        position.case_sensitive = case_sensitive;
        return position;
    }

    static std::vector<SPAN> points(std::initializer_list<size_t> indexes)
    {
        std::vector<SPAN> result;
        for (const size_t index : indexes) {
            result.push_back({index, index});
        }
        return result;
    }

    std::unique_ptr<TEXT_SERVICE> text_service_ = PLATFORM::create_text_service();
    SPAN_FINDER finder_{*text_service_};
};
