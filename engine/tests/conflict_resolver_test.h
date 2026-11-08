#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "conflict.h"
#include "conflict_entry.h"
#include "conflict_resolver.h"
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
