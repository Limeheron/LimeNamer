#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "conflict.h"
#include "conflict_entry.h"
#include "conflict_resolver.h"
#include "duplicate_options.h"
#include "fake_fs_env.h"
#include "folder_cache.h"

class CONFLICT_RESOLVER_TEST : public testing::Test
{
protected:
    void SetUp() override
    {
        env_.make_folder(env_.root() / "a");
        env_.make_folder(env_.root() / "b");
    }

    CONFLICT_ENTRY entry(uint64_t id, const char *folder, std::u32string current,
        std::u32string next)
    {
        CONFLICT_ENTRY value;
        value.id = id;
        value.folder = env_.root() / folder;
        value.changed = current != next;
        value.current_name = std::move(current);
        value.new_name = std::move(next);
        return value;
    }

    void existing(const char *folder, const std::filesystem::path &name)
    {
        env_.make_file(env_.root() / folder / name, 1);
    }

    std::vector<CONFLICT> find(const std::vector<CONFLICT_ENTRY> &entries)
    {
        cache_.load(env_.root() / "a");
        cache_.load(env_.root() / "b");
        return resolver_.find(entries, cache_);
    }

    std::vector<std::u32string> adjust(const std::vector<CONFLICT_ENTRY> &entries,
        DUPLICATE_MODE mode = DUPLICATE_MODE::keep_first,
        DUPLICATE_FORMAT format = DUPLICATE_FORMAT::paren, std::u32string custom_text = U"")
    {
        cache_.load(env_.root() / "a");
        cache_.load(env_.root() / "b");
        DUPLICATE_OPTIONS options;
        options.enabled = true;
        options.mode = mode;
        options.format = format;
        options.custom_text = std::move(custom_text);
        const std::vector<std::optional<std::u32string>> adjusted =
            resolver_.adjust(entries, cache_, options);
        std::vector<std::u32string> names;
        std::vector<CONFLICT_ENTRY> after = entries;
        for (size_t index = 0; index < entries.size(); ++index) {
            names.push_back(adjusted[index].value_or(entries[index].new_name));
            after[index].new_name = names.back();
            after[index].changed = after[index].current_name != after[index].new_name;
        }
        for (const CONFLICT &conflict : resolver_.find(after, cache_)) {
            EXPECT_EQ(conflict, none());
        }
        return names;
    }

    static CONFLICT none()
    {
        return CONFLICT();
    }

    static CONFLICT with_item(uint64_t id)
    {
        return {CONFLICT_KIND::list_item, id};
    }

    static CONFLICT with_existing()
    {
        return {CONFLICT_KIND::existing_item, 0};
    }

    FAKE_FS_ENV env_;
    FOLDER_CACHE cache_{env_.file_system(), env_.text_service()};
    CONFLICT_RESOLVER resolver_{env_.text_service()};
};
