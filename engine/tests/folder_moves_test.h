#pragma once

#include <filesystem>
#include <optional>

#include <gtest/gtest.h>

#include "folder_moves.h"

class FOLDER_MOVES_TEST : public testing::Test
{
protected:
    static std::filesystem::path at(const std::filesystem::path &relative)
    {
        return std::filesystem::path("/root") / relative;
    }

    FOLDER_MOVES moves_;
};
