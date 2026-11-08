#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

#include <gtest/gtest.h>

#include "history.h"
#include "snapshot.h"

class HISTORY_TEST : public testing::Test
{
protected:
    static SNAPSHOT state(uint64_t value)
    {
        SNAPSHOT snapshot;
        snapshot.order = {value};
        return snapshot;
    }

    HISTORY history_;
};
