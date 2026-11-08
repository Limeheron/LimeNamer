#include "c_api.h"

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
