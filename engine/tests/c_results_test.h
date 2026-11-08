#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "add_result.h"
#include "c_results.h"
#include "example.h"
#include "index_list_result.h"
#include "limenamer.h"
#include "mod_problem.h"
#include "name_issue.h"
#include "preview.h"
#include "rename_report.h"
#include "utf.h"

class C_RESULTS_TEST : public testing::Test
{
protected:
    template <typename T>
    using OWNED = std::unique_ptr<T, void (*)(void *)>;

    template <typename T>
    static OWNED<T> owned(T *value)
    {
        return OWNED<T>(value, &C_RESULTS::release);
    }

    static PREVIEW_ROW row(uint64_t id, const std::u32string &folder, const std::u32string &name,
        const std::u32string &new_name)
    {
        PREVIEW_ROW value;
        value.item_id = id;
        value.folder = UTF::path_from_u32(folder);
        value.name = name;
        value.new_name = new_name;
        return value;
    }
};
