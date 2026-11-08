#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "fake_fs_env.h"
#include "folder_cache.h"
#include "item_store.h"
#include "mod_json.h"
#include "mod_store.h"
#include "name_checker.h"
#include "name_issue.h"
#include "options.h"
#include "preview.h"
#include "preview_builder.h"
#include "utf.h"

class PREVIEW_BUILDER_TEST : public testing::Test
{
protected:
    void SetUp() override
    {
        env_.make_folder(env_.root() / "photos");
        env_.make_folder(env_.root() / "docs");
    }

    std::vector<uint64_t> add_files(const char *folder, const std::vector<std::u32string> &names)
    {
        std::vector<std::filesystem::path> paths;
        for (const std::u32string &name : names) {
            const std::filesystem::path path = env_.root() / folder / UTF::path_from_u32(name);
            env_.make_file(path, 10);
            paths.push_back(path);
        }
        const size_t before = items_.size();
        items_.add(paths, ADD_MODE::as_items);
        cache_.load(env_.root() / folder);
        return std::vector<uint64_t>(items_.order().begin() + static_cast<std::ptrdiff_t>(before),
            items_.order().end());
    }

    uint64_t add_mod(const std::string &json)
    {
        return mods_.add(MOD_JSON::parse(json));
    }

    PREVIEW build() const
    {
        return builder_.build(items_, mods_.entries(), options_, cache_);
    }

    std::vector<std::u32string> new_names() const
    {
        std::vector<std::u32string> names;
        for (const PREVIEW_ROW &row : build().rows) {
            names.push_back(row.new_name);
        }
        return names;
    }

    FAKE_FS_ENV env_;
    ITEM_STORE items_{env_.file_system(), env_.text_service()};
    MOD_STORE mods_{env_.text_service()};
    FOLDER_CACHE cache_{env_.file_system(), env_.text_service()};
    NAME_CHECKER checker_{true};
    PREVIEW_BUILDER builder_{env_.text_service(), checker_};
    OPTIONS options_;
};
