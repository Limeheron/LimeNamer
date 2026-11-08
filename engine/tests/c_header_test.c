#include "c_header_test.h"

static const char *const PREFIX =
    "{\"kind\":\"insert\",\"position\":{\"type\":\"start\"},\"value\":[{\"text\":\"a_\"}]}";

static const char *const UPPER =
    "{\"kind\":\"format\",\"position\":{\"type\":\"all\"},\"format\":{\"type\":\"upper\"}}";

static const char *const EMPTY_REPLACE =
    "{\"kind\":\"replace\",\"position\":{\"type\":\"text\",\"text\":\"\"},"
    "\"value\":[{\"text\":\"\"}]}";

static int failure_count = 0;

static void expect(int condition, const char *step)
{
    if (!condition) {
        fprintf(stderr, "failed: %s\n", step);
        ++failure_count;
    }
}

static void expect_result(LN_RESULT actual, LN_RESULT expected, const char *step)
{
    if (actual != expected) {
        fprintf(stderr, "failed: %s (result %d, expected %d)\n", step, (int)actual,
            (int)expected);
        ++failure_count;
    }
}

static int contains(const char *text, const char *part)
{
    return text != NULL && strstr(text, part) != NULL;
}

static int is_prefixed(const char *name, const char *base)
{
    return name != NULL && base != NULL && strncmp(name, "a_", 2) == 0
        && strcmp(name + 2, base) == 0;
}

static void count_progress(void *user, uint32_t done, uint32_t total)
{
    uint32_t *calls = (uint32_t *)user;
    (void)done;
    (void)total;
    ++*calls;
}

static void check_without_session(void)
{
    const char *version = ln_version();
    expect(version != NULL && version[0] != '\0', "ln_version");

    LN_INDEX_LIST *list = NULL;
    expect_result(ln_index_list_parse("3-5, 1", 10, &list), ln_result_ok,
        "ln_index_list_parse");
    expect(list != NULL && list->count == 4 && list->error == ln_index_error_none
        && list->indices[0] == 1 && list->indices[3] == 5, "index list values");

    char *text = NULL;
    if (list != NULL) {
        expect_result(ln_index_list_format(list->indices, list->count, &text), ln_result_ok,
            "ln_index_list_format");
    }
    expect(text != NULL && strcmp(text, "1, 3-5") == 0, "formatted index list");
    ln_string_free(text);
    ln_index_list_free(list);

    ln_string_free(NULL);
    ln_problems_free(NULL);
    ln_example_free(NULL);
    ln_preview_free(NULL);
    ln_report_free(NULL);
    ln_index_list_free(NULL);
    ln_session_destroy(NULL);
}

static void check_empty_rename(void)
{
    LN_SESSION *session = NULL;
    expect_result(ln_session_create(&session), ln_result_ok, "ln_session_create");
    if (session == NULL) {
        return;
    }

    uint32_t calls = 0;
    LN_REPORT *report = NULL;
    expect_result(ln_rename(session, count_progress, &calls, &report), ln_result_ok,
        "ln_rename");
    expect(report != NULL && report->total == 0 && report->problem_count == 0
        && report->problems == NULL && report->needs_choice == 0, "empty rename report");
    ln_report_free(report);

    expect_result(ln_rename_resolve(session, ln_resolve_choice_revert_all, NULL, NULL, &report),
        ln_result_not_found, "ln_rename_resolve");
    expect(report == NULL, "resolve report");

    expect_result(ln_rename_undo_check(session, &report), ln_result_ok, "ln_rename_undo_check");
    expect(report != NULL && report->total == 0, "undo check report");
    ln_report_free(report);

    expect_result(ln_rename_undo(session, count_progress, &calls, &report),
        ln_result_not_ready, "ln_rename_undo");
    expect(report == NULL, "undo report");

    uint8_t available = 9;
    uint32_t count = 9;
    expect_result(ln_rename_undo_state(session, &available, &count), ln_result_ok,
        "ln_rename_undo_state");
    expect(available == 0 && count == 0, "undo state values");
    expect(calls == 0, "progress calls");
    ln_session_destroy(session);
}

static void check_items(LN_SESSION *session, const char *path, uint64_t *item_id)
{
    const char *paths[1];
    paths[0] = path;
    LN_ADD_RESULT added = {0, 0, 0};
    expect_result(ln_items_add(session, paths, 1, ln_add_mode_as_items, &added), ln_result_ok,
        "ln_items_add");
    expect(added.added == 1 && added.skipped == 0 && added.failed == 0, "added counts");
    expect_result(ln_items_add(session, paths, 1, ln_add_mode_as_items, &added), ln_result_ok,
        "ln_items_add again");
    expect(added.added == 0 && added.skipped == 1, "skipped counts");

    LN_PREVIEW *preview = NULL;
    expect_result(ln_preview(session, &preview), ln_result_ok, "ln_preview");
    if (preview == NULL || preview->count != 1) {
        expect(0, "preview rows");
        ln_preview_free(preview);
        return;
    }
    const LN_ROW *row = &preview->rows[0];
    *item_id = row->item_id;
    expect(row->folder != NULL && row->name != NULL && strcmp(row->name, row->new_name) == 0,
        "unchanged row names");
    expect(row->size > 0 && row->is_dir == 0 && row->changed == 0, "unchanged row values");
    expect(row->status == ln_status_unchanged && row->conflict == ln_conflict_none,
        "unchanged row status");
    ln_preview_free(preview);

    uint8_t changed = 9;
    expect_result(ln_items_move(session, item_id, 1, *item_id, 0, 1, &changed), ln_result_ok,
        "ln_items_move");
    expect(changed == 0, "move result");
    expect_result(ln_items_sort(session, ln_sort_key_name_natural, 1), ln_result_ok,
        "ln_items_sort");
    int32_t key = -1;
    uint8_t descending = 9;
    expect_result(ln_items_sort_state(session, &key, &descending), ln_result_ok,
        "ln_items_sort_state");
    expect(key == ln_sort_key_name_natural && descending == 1, "sort state values");
    expect_result(ln_folders_refresh(session), ln_result_ok, "ln_folders_refresh");
}

static void check_mods(LN_SESSION *session, uint64_t item_id)
{
    uint64_t prefix = 0;
    uint64_t upper = 0;
    expect_result(ln_mod_add(session, PREFIX, &prefix), ln_result_ok, "ln_mod_add");
    expect_result(ln_mod_add(session, UPPER, &upper), ln_result_ok, "ln_mod_add upper");
    expect_result(ln_mod_update(session, prefix, PREFIX), ln_result_ok, "ln_mod_update");
    uint8_t changed = 9;
    expect_result(ln_mod_move(session, upper, prefix, 0, 1, &changed), ln_result_ok,
        "ln_mod_move");
    expect(changed == 1, "mod move result");
    expect_result(ln_mod_set_enabled(session, upper, 0), ln_result_ok, "ln_mod_set_enabled");
    expect_result(ln_mods_set_all_enabled(session, 1), ln_result_ok,
        "ln_mods_set_all_enabled");
    expect_result(ln_mod_remove(session, upper), ln_result_ok, "ln_mod_remove");

    char *json = NULL;
    expect_result(ln_mods_get(session, &json), ln_result_ok, "ln_mods_get");
    expect(contains(json, "\"kind\":\"insert\"") && !contains(json, "\"kind\":\"format\""),
        "mods json");
    ln_string_free(json);

    LN_PROBLEMS *problems = NULL;
    expect_result(ln_mod_check(session, EMPTY_REPLACE, &problems), ln_result_ok,
        "ln_mod_check");
    expect(problems != NULL && problems->count == 2
        && problems->items[1].field == ln_mod_field_value
        && problems->items[1].reason == ln_problem_reason_required, "mod problems");
    ln_problems_free(problems);

    LN_EXAMPLE *example = NULL;
    expect_result(ln_mod_example(session, PREFIX, 0, &example), ln_result_ok,
        "ln_mod_example");
    expect(example != NULL && example->item_id == item_id && example->index == 1
        && example->state == ln_example_state_changed
        && is_prefixed(example->after, example->before), "example values");
    ln_example_free(example);

    LN_PREVIEW *preview = NULL;
    expect_result(ln_preview(session, &preview), ln_result_ok, "ln_preview with mod");
    expect(preview != NULL && preview->count == 1 && preview->changed == 1
        && preview->runnable == 1 && preview->rows[0].changed == 1
        && preview->rows[0].status == ln_status_ok
        && is_prefixed(preview->rows[0].new_name, preview->rows[0].name), "changed row");
    ln_preview_free(preview);

    expect_result(ln_options_get(session, &json), ln_result_ok, "ln_options_get");
    expect(contains(json, "\"duplicate\""), "options json");
    ln_string_free(json);
    expect_result(ln_options_set(session, "{\"duplicate\":{\"enabled\":true}}"), ln_result_ok,
        "ln_options_set");

    expect_result(ln_mod_direct_edit(session, item_id, "c_header_test"), ln_result_ok,
        "ln_mod_direct_edit");
    expect_result(ln_preview(session, &preview), ln_result_ok, "ln_preview after edit");
    expect(preview != NULL && preview->count == 1 && preview->rows[0].direct == 1
        && strcmp(preview->rows[0].new_name, "c_header_test") == 0, "edited row");
    ln_preview_free(preview);
}

static void check_history(LN_SESSION *session, uint64_t item_id)
{
    uint8_t can_undo = 9;
    uint8_t can_redo = 9;
    expect_result(ln_history_state(session, &can_undo, &can_redo), ln_result_ok,
        "ln_history_state");
    expect(can_undo == 1 && can_redo == 0, "history before undo");
    expect_result(ln_undo(session), ln_result_ok, "ln_undo");
    expect_result(ln_history_state(session, &can_undo, &can_redo), ln_result_ok,
        "ln_history_state after undo");
    expect(can_redo == 1, "history after undo");
    expect_result(ln_redo(session), ln_result_ok, "ln_redo");

    expect_result(ln_items_remove(session, &item_id, 1), ln_result_ok, "ln_items_remove");
    expect_result(ln_items_clear(session), ln_result_ok, "ln_items_clear");
}

static void check_session(const char *path)
{
    LN_SESSION *session = NULL;
    expect_result(ln_session_create(&session), ln_result_ok, "ln_session_create");
    if (session == NULL) {
        return;
    }
    uint64_t item_id = 0;
    check_items(session, path, &item_id);
    if (item_id != 0) {
        check_mods(session, item_id);
        check_history(session, item_id);
    }
    ln_session_destroy(session);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "failed: expected one file path\n");
        return 1;
    }
    check_without_session();
    check_empty_rename();
    check_session(argv[1]);
    if (failure_count > 0) {
        fprintf(stderr, "%d checks failed\n", failure_count);
        return 1;
    }
    return 0;
}
