#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "example.h"
#include "example_finder.h"
#include "fake_fs_env.h"
#include "item_store.h"
#include "mod_json.h"
#include "mod_store.h"
#include "name_checker.h"
#include "name_issue.h"
#include "preview_builder.h"
#include "utf.h"

class EXAMPLE_FINDER_TEST : public testing::Test
{
protected:
    std::vector<uint64_t> add(const std::vector<std::u32string> &names, bool folders = false)
    {
        std::vector<std::filesystem::path> paths;
        for (const std::u32string &name : names) {
            const std::filesystem::path path = env_.root() / UTF::path_from_u32(name);
            if (folders) {
                env_.make_folder(path);
            } else {
                env_.make_file(path, 1);
            }
            paths.push_back(path);
        }
        const size_t before = items_.size();
        items_.add(paths, ADD_MODE::as_items);
        return std::vector<uint64_t>(items_.order().begin() + static_cast<std::ptrdiff_t>(before),
            items_.order().end());
    }

    EXAMPLE example(const std::string &draft, uint64_t before_mod_id = 0) const
    {
        return finder_.find(items_, mods_.entries(), MOD_JSON::parse(draft), before_mod_id)
            .value();
    }

    FAKE_FS_ENV env_;
    ITEM_STORE items_{env_.file_system(), env_.text_service()};
    MOD_STORE mods_{env_.text_service()};
    NAME_CHECKER checker_{true};
    PREVIEW_BUILDER builder_{env_.text_service(), checker_};
    EXAMPLE_FINDER finder_{builder_, checker_};
};
