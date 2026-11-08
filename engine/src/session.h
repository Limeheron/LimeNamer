#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

#include "add_mode.h"
#include "add_result.h"
#include "example.h"
#include "example_finder.h"
#include "file_system.h"
#include "folder_cache.h"
#include "history.h"
#include "item_store.h"
#include "mod_entry.h"
#include "mod_store.h"
#include "modification.h"
#include "name_checker.h"
#include "options.h"
#include "preview.h"
#include "preview_builder.h"
#include "snapshot.h"
#include "sort_key.h"
#include "sort_state.h"
#include "text_service.h"

class SESSION
{
public:
    static constexpr const char VERSION_TOKEN[] = "LIMENAMER_VERSION=0.1.0";

    SESSION(std::unique_ptr<TEXT_SERVICE> text_service, std::unique_ptr<FILE_SYSTEM> file_system,
        bool windows_host);
    SESSION(const SESSION &) = delete;
    SESSION &operator=(const SESSION &) = delete;

    static const char *version();

    const TEXT_SERVICE &text_service() const;
    FILE_SYSTEM &file_system();

    ADD_RESULT add_items(const std::vector<std::filesystem::path> &paths, ADD_MODE mode);
    size_t remove_items(const std::vector<uint64_t> &ids);
    void clear_items();
    std::optional<bool> move_items(const std::vector<uint64_t> &ids, uint64_t target_id,
        bool after, bool dry_run);
    void sort_items(SORT_KEY key, bool descending);
    const SORT_STATE &sort_state() const;
    void refresh_folders();
    const ITEM_STORE &items() const;

    const std::vector<MOD_ENTRY> &mods() const;
    uint64_t add_mod(MODIFICATION mod);
    bool update_mod(uint64_t id, MODIFICATION mod);
    bool remove_mod(uint64_t id);
    std::optional<bool> move_mod(uint64_t id, uint64_t target_id, bool after, bool dry_run);
    bool set_mod_enabled(uint64_t id, bool enabled);
    void set_all_mods_enabled(bool enabled);
    std::optional<uint64_t> direct_edit(uint64_t item_id, std::u32string new_name);
    std::optional<EXAMPLE> example(const MODIFICATION &draft, uint64_t before_mod_id) const;

    const OPTIONS &options() const;
    void set_options(OPTIONS options);

    PREVIEW preview() const;

    bool undo();
    bool redo();
    bool can_undo() const;
    bool can_redo() const;

private:
    template <typename SERVICE>
    static std::unique_ptr<SERVICE> required(std::unique_ptr<SERVICE> service)
    {
        if (service == nullptr) {
            throw std::invalid_argument("session requires text service and file system");
        }
        return service;
    }

    SNAPSHOT snapshot() const;
    void restore(SNAPSHOT snapshot);
    void track_folders();

    template <typename BODY>
    auto recorded(BODY &&body)
    {
        SNAPSHOT before = snapshot();
        if constexpr (std::is_void_v<decltype(body())>) {
            body();
            history_.record(std::move(before), snapshot());
        } else {
            auto result = body();
            history_.record(std::move(before), snapshot());
            return result;
        }
    }

    std::unique_ptr<TEXT_SERVICE> text_service_;
    std::unique_ptr<FILE_SYSTEM> file_system_;
    NAME_CHECKER checker_;
    ITEM_STORE items_;
    MOD_STORE mods_;
    FOLDER_CACHE cache_;
    OPTIONS options_;
    HISTORY history_;
    PREVIEW_BUILDER builder_;
    EXAMPLE_FINDER finder_;
};
