#include "c_api.h"

namespace
{
RENAME_EXECUTOR::PROGRESS progress_of(LN_PROGRESS_FN progress, void *user)
{
    if (progress == nullptr) {
        return {};
    }
    return [progress, user](uint32_t done, uint32_t total) {
        progress(user, done, total);
    };
}
}

LN_RESULT ln_session_create(LN_SESSION **out)
{
    if (out == nullptr) {
        return ln_result_invalid_argument;
    }
    *out = nullptr;
    return C_GUARD::run([out]() -> LN_RESULT {
        *out = std::make_unique<LN_SESSION>(PLATFORM::create_text_service(),
            PLATFORM::create_file_system(), PLATFORM::WINDOWS_HOST).release();
        return ln_result_ok;
    });
}

void ln_session_destroy(LN_SESSION *session)
{
    delete session;
}

const char *ln_version(void)
{
    return SESSION::version();
}

LN_RESULT ln_items_add(LN_SESSION *session, const char *const *paths, size_t count,
    int32_t mode, LN_ADD_RESULT *out)
{
    return C_GUARD::run(session, [&](SESSION &target) -> LN_RESULT {
        const std::optional<std::vector<std::filesystem::path>> values =
            C_INPUT::paths(paths, count);
        const std::optional<ADD_MODE> add_mode = C_INPUT::choice(mode, ADD_MODE::folder_contents);
        if (out == nullptr || !values || !add_mode) {
            return ln_result_invalid_argument;
        }
        *out = C_RESULTS::add_result(target.add_items(*values, *add_mode));
        return ln_result_ok;
    });
}

LN_RESULT ln_items_remove(LN_SESSION *session, const uint64_t *ids, size_t count)
{
    return C_GUARD::run(session, [&](SESSION &target) -> LN_RESULT {
        const std::optional<std::vector<uint64_t>> values = C_INPUT::ids(ids, count);
        if (!values) {
            return ln_result_invalid_argument;
        }
        for (const uint64_t id : *values) {
            if (!target.items().contains(id)) {
                return ln_result_not_found;
            }
        }
        target.remove_items(*values);
        return ln_result_ok;
    });
}

LN_RESULT ln_items_clear(LN_SESSION *session)
{
    return C_GUARD::run(session, [](SESSION &target) -> LN_RESULT {
        target.clear_items();
        return ln_result_ok;
    });
}

LN_RESULT ln_items_move(LN_SESSION *session, const uint64_t *ids, size_t count,
    uint64_t target_id, uint8_t after, uint8_t dry_run, uint8_t *changed)
{
    return C_GUARD::run(session, [&](SESSION &target) -> LN_RESULT {
        const std::optional<std::vector<uint64_t>> values = C_INPUT::ids(ids, count);
        const std::optional<bool> after_target = C_INPUT::flag(after);
        const std::optional<bool> only_check = C_INPUT::flag(dry_run);
        if (changed == nullptr || !values || values->empty() || !after_target || !only_check) {
            return ln_result_invalid_argument;
        }
        const std::optional<bool> moved =
            target.move_items(*values, target_id, *after_target, *only_check);
        if (!moved) {
            return ln_result_not_found;
        }
        *changed = *moved ? 1 : 0;
        return ln_result_ok;
    });
}

LN_RESULT ln_items_sort(LN_SESSION *session, int32_t key, uint8_t descending)
{
    return C_GUARD::run(session, [&](SESSION &target) -> LN_RESULT {
        const std::optional<SORT_KEY> sort_key = C_INPUT::choice(key, SORT_KEY::modified);
        const std::optional<bool> reversed = C_INPUT::flag(descending);
        if (!sort_key || !reversed) {
            return ln_result_invalid_argument;
        }
        target.sort_items(*sort_key, *reversed);
        return ln_result_ok;
    });
}

LN_RESULT ln_items_sort_state(LN_SESSION *session, int32_t *key, uint8_t *descending)
{
    return C_GUARD::run(session, [&](SESSION &target) -> LN_RESULT {
        if (key == nullptr || descending == nullptr) {
            return ln_result_invalid_argument;
        }
        const SORT_STATE &state = target.sort_state();
        *key = static_cast<int32_t>(state.key);
        *descending = state.descending ? 1 : 0;
        return ln_result_ok;
    });
}

LN_RESULT ln_folders_refresh(LN_SESSION *session)
{
    return C_GUARD::run(session, [](SESSION &target) -> LN_RESULT {
        target.refresh_folders();
        return ln_result_ok;
    });
}

LN_RESULT ln_mods_get(LN_SESSION *session, char **json)
{
    if (json != nullptr) {
        *json = nullptr;
    }
    return C_GUARD::run(session, [&](SESSION &target) -> LN_RESULT {
        if (json == nullptr) {
            return ln_result_invalid_argument;
        }
        *json = C_RESULTS::string(MOD_JSON::entries_to_json(target.mods()));
        return ln_result_ok;
    });
}

LN_RESULT ln_mod_add(LN_SESSION *session, const char *json, uint64_t *id)
{
    return C_GUARD::run(session, [&](SESSION &target) -> LN_RESULT {
        if (json == nullptr || id == nullptr) {
            return ln_result_invalid_argument;
        }
        *id = target.add_mod(MOD_JSON::parse(json));
        return ln_result_ok;
    });
}

LN_RESULT ln_mod_update(LN_SESSION *session, uint64_t id, const char *json)
{
    return C_GUARD::run(session, [&](SESSION &target) -> LN_RESULT {
        if (json == nullptr) {
            return ln_result_invalid_argument;
        }
        return target.update_mod(id, MOD_JSON::parse(json)) ? ln_result_ok : ln_result_not_found;
    });
}

LN_RESULT ln_mod_remove(LN_SESSION *session, uint64_t id)
{
    return C_GUARD::run(session, [&](SESSION &target) -> LN_RESULT {
        return target.remove_mod(id) ? ln_result_ok : ln_result_not_found;
    });
}

LN_RESULT ln_mod_move(LN_SESSION *session, uint64_t id, uint64_t target_id, uint8_t after,
    uint8_t dry_run, uint8_t *changed)
{
    return C_GUARD::run(session, [&](SESSION &target) -> LN_RESULT {
        const std::optional<bool> after_target = C_INPUT::flag(after);
        const std::optional<bool> only_check = C_INPUT::flag(dry_run);
        if (changed == nullptr || !after_target || !only_check) {
            return ln_result_invalid_argument;
        }
        const std::optional<bool> moved =
            target.move_mod(id, target_id, *after_target, *only_check);
        if (!moved) {
            return ln_result_not_found;
        }
        *changed = *moved ? 1 : 0;
        return ln_result_ok;
    });
}

LN_RESULT ln_mod_set_enabled(LN_SESSION *session, uint64_t id, uint8_t enabled)
{
    return C_GUARD::run(session, [&](SESSION &target) -> LN_RESULT {
        const std::optional<bool> value = C_INPUT::flag(enabled);
        if (!value) {
            return ln_result_invalid_argument;
        }
        return target.set_mod_enabled(id, *value) ? ln_result_ok : ln_result_not_found;
    });
}

LN_RESULT ln_mods_set_all_enabled(LN_SESSION *session, uint8_t enabled)
{
    return C_GUARD::run(session, [&](SESSION &target) -> LN_RESULT {
        const std::optional<bool> value = C_INPUT::flag(enabled);
        if (!value) {
            return ln_result_invalid_argument;
        }
        target.set_all_mods_enabled(*value);
        return ln_result_ok;
    });
}

LN_RESULT ln_mod_direct_edit(LN_SESSION *session, uint64_t item_id, const char *new_name)
{
    return C_GUARD::run(session, [&](SESSION &target) -> LN_RESULT {
        std::optional<std::u32string> name = C_INPUT::text(new_name);
        if (!name) {
            return ln_result_invalid_argument;
        }
        if (!target.direct_edit(item_id, std::move(*name))) {
            return ln_result_not_found;
        }
        return ln_result_ok;
    });
}

LN_RESULT ln_mod_check(LN_SESSION *session, const char *json, LN_PROBLEMS **out)
{
    if (out != nullptr) {
        *out = nullptr;
    }
    return C_GUARD::run(session, [&](SESSION &) -> LN_RESULT {
        if (json == nullptr || out == nullptr) {
            return ln_result_invalid_argument;
        }
        *out = C_RESULTS::problems(MOD_CHECKER::check(MOD_JSON::parse(json)));
        return ln_result_ok;
    });
}

LN_RESULT ln_mod_example(LN_SESSION *session, const char *json, uint64_t before_mod_id,
    LN_EXAMPLE **out)
{
    if (out != nullptr) {
        *out = nullptr;
    }
    return C_GUARD::run(session, [&](SESSION &target) -> LN_RESULT {
        if (json == nullptr || out == nullptr) {
            return ln_result_invalid_argument;
        }
        const std::optional<EXAMPLE> example =
            target.example(MOD_JSON::parse(json), before_mod_id);
        if (!example) {
            return ln_result_not_found;
        }
        *out = C_RESULTS::example(*example);
        return ln_result_ok;
    });
}

LN_RESULT ln_mod_sample(LN_SESSION *session, const char *json, LN_EXAMPLE **out)
{
    if (out != nullptr) {
        *out = nullptr;
    }
    return C_GUARD::run(session, [&](SESSION &target) -> LN_RESULT {
        if (json == nullptr || out == nullptr) {
            return ln_result_invalid_argument;
        }
        *out = C_RESULTS::example(target.sample(MOD_JSON::parse(json)));
        return ln_result_ok;
    });
}

LN_RESULT ln_options_get(LN_SESSION *session, char **json)
{
    if (json != nullptr) {
        *json = nullptr;
    }
    return C_GUARD::run(session, [&](SESSION &target) -> LN_RESULT {
        if (json == nullptr) {
            return ln_result_invalid_argument;
        }
        *json = C_RESULTS::string(MOD_JSON::options_to_json(target.options()));
        return ln_result_ok;
    });
}

LN_RESULT ln_options_set(LN_SESSION *session, const char *json)
{
    return C_GUARD::run(session, [&](SESSION &target) -> LN_RESULT {
        if (json == nullptr) {
            return ln_result_invalid_argument;
        }
        target.set_options(MOD_JSON::parse_options(json));
        return ln_result_ok;
    });
}

LN_RESULT ln_preview(LN_SESSION *session, LN_PREVIEW **out)
{
    if (out != nullptr) {
        *out = nullptr;
    }
    return C_GUARD::run(session, [&](SESSION &target) -> LN_RESULT {
        if (out == nullptr) {
            return ln_result_invalid_argument;
        }
        *out = C_RESULTS::preview(target.preview());
        return ln_result_ok;
    });
}

LN_RESULT ln_undo(LN_SESSION *session)
{
    return C_GUARD::run(session, [](SESSION &target) -> LN_RESULT {
        return target.undo() ? ln_result_ok : ln_result_not_found;
    });
}

LN_RESULT ln_redo(LN_SESSION *session)
{
    return C_GUARD::run(session, [](SESSION &target) -> LN_RESULT {
        return target.redo() ? ln_result_ok : ln_result_not_found;
    });
}

LN_RESULT ln_history_state(LN_SESSION *session, uint8_t *can_undo, uint8_t *can_redo)
{
    return C_GUARD::run(session, [&](SESSION &target) -> LN_RESULT {
        if (can_undo == nullptr || can_redo == nullptr) {
            return ln_result_invalid_argument;
        }
        *can_undo = target.can_undo() ? 1 : 0;
        *can_redo = target.can_redo() ? 1 : 0;
        return ln_result_ok;
    });
}

LN_RESULT ln_rename(LN_SESSION *session, LN_PROGRESS_FN progress, void *user, LN_REPORT **out)
{
    if (out != nullptr) {
        *out = nullptr;
    }
    return C_GUARD::run(session, [&](SESSION &target) -> LN_RESULT {
        if (out == nullptr) {
            return ln_result_invalid_argument;
        }
        const std::optional<RENAME_REPORT> report = target.rename(progress_of(progress, user));
        if (!report) {
            return ln_result_not_ready;
        }
        *out = C_RESULTS::report(*report);
        return ln_result_ok;
    });
}

LN_RESULT ln_rename_resolve(LN_SESSION *session, int32_t choice, LN_PROGRESS_FN progress,
    void *user, LN_REPORT **out)
{
    if (out != nullptr) {
        *out = nullptr;
    }
    return C_GUARD::run(session, [&](SESSION &target) -> LN_RESULT {
        const std::optional<RESOLVE_CHOICE> value =
            C_INPUT::choice(choice, RESOLVE_CHOICE::keep_succeeded);
        if (out == nullptr || !value) {
            return ln_result_invalid_argument;
        }
        if (!target.rename_pending()) {
            return ln_result_not_found;
        }
        *out = C_RESULTS::report(target.resolve_rename(*value, progress_of(progress, user)));
        return ln_result_ok;
    });
}

LN_RESULT ln_rename_undo_check(LN_SESSION *session, LN_REPORT **out)
{
    if (out != nullptr) {
        *out = nullptr;
    }
    return C_GUARD::run(session, [&](SESSION &target) -> LN_RESULT {
        if (out == nullptr) {
            return ln_result_invalid_argument;
        }
        *out = C_RESULTS::report(target.undo_check());
        return ln_result_ok;
    });
}

LN_RESULT ln_rename_undo(LN_SESSION *session, LN_PROGRESS_FN progress, void *user,
    LN_REPORT **out)
{
    if (out != nullptr) {
        *out = nullptr;
    }
    return C_GUARD::run(session, [&](SESSION &target) -> LN_RESULT {
        if (out == nullptr) {
            return ln_result_invalid_argument;
        }
        const std::optional<RENAME_REPORT> report =
            target.undo_rename(progress_of(progress, user));
        if (!report) {
            return ln_result_not_ready;
        }
        *out = C_RESULTS::report(*report);
        return ln_result_ok;
    });
}

LN_RESULT ln_rename_undo_state(LN_SESSION *session, uint8_t *available, uint32_t *count)
{
    return C_GUARD::run(session, [&](SESSION &target) -> LN_RESULT {
        if (available == nullptr || count == nullptr) {
            return ln_result_invalid_argument;
        }
        *available = target.record().empty() ? 0 : 1;
        *count = static_cast<uint32_t>(target.record().size());
        return ln_result_ok;
    });
}

LN_RESULT ln_index_list_parse(const char *text, uint32_t item_count, LN_INDEX_LIST **out)
{
    if (out != nullptr) {
        *out = nullptr;
    }
    return C_GUARD::run([&]() -> LN_RESULT {
        const std::optional<std::u32string> value = C_INPUT::text(text);
        if (!value || out == nullptr) {
            return ln_result_invalid_argument;
        }
        *out = C_RESULTS::index_list(INDEX_LIST::parse(*value, item_count));
        return ln_result_ok;
    });
}

LN_RESULT ln_index_list_format(const uint32_t *indices, size_t count, char **out)
{
    if (out != nullptr) {
        *out = nullptr;
    }
    return C_GUARD::run([&]() -> LN_RESULT {
        if (out == nullptr || (indices == nullptr && count > 0)) {
            return ln_result_invalid_argument;
        }
        std::vector<uint32_t> values;
        if (count > 0) {
            values.assign(indices, indices + count);
        }
        for (const uint32_t value : values) {
            if (value == 0) {
                return ln_result_invalid_argument;
            }
        }
        *out = C_RESULTS::string(UTF::encode_utf8(INDEX_LIST::format(std::move(values))));
        return ln_result_ok;
    });
}

void ln_string_free(char *text)
{
    C_RESULTS::release(text);
}

void ln_problems_free(LN_PROBLEMS *problems)
{
    C_RESULTS::release(problems);
}

void ln_example_free(LN_EXAMPLE *example)
{
    C_RESULTS::release(example);
}

void ln_preview_free(LN_PREVIEW *preview)
{
    C_RESULTS::release(preview);
}

void ln_report_free(LN_REPORT *report)
{
    C_RESULTS::release(report);
}

void ln_index_list_free(LN_INDEX_LIST *list)
{
    C_RESULTS::release(list);
}
