#pragma once

#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32)
#if defined(LN_BUILDING)
#define LN_API __declspec(dllexport)
#else
#define LN_API __declspec(dllimport)
#endif
#else
#define LN_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef int32_t LN_RESULT;

enum
{
    ln_result_ok = 0,
    ln_result_invalid_argument = 1,
    ln_result_invalid_json = 2,
    ln_result_not_found = 3,
    ln_result_busy = 4,
    ln_result_not_ready = 5,
    ln_result_io = 6,
    ln_result_out_of_memory = 7,
    ln_result_internal = 8
};

typedef int32_t LN_STATUS;

enum
{
    ln_status_ok = 0,
    ln_status_unchanged = 1,
    ln_status_warning = 2,
    ln_status_error = 3,
    ln_status_conflict = 4
};

typedef uint32_t LN_NAME_ISSUE;

enum
{
    ln_name_issue_empty = 1 << 0,
    ln_name_issue_dot_name = 1 << 1,
    ln_name_issue_win_forbidden_char = 1 << 2,
    ln_name_issue_win_reserved = 1 << 3,
    ln_name_issue_win_trailing = 1 << 4,
    ln_name_issue_win_name_too_long = 1 << 5,
    ln_name_issue_win_path_too_long = 1 << 6,
    ln_name_issue_mac_forbidden_char = 1 << 7,
    ln_name_issue_mac_name_too_long = 1 << 8,
    ln_name_issue_hidden = 1 << 9
};

typedef int32_t LN_CONFLICT;

enum
{
    ln_conflict_none = 0,
    ln_conflict_list_item = 1,
    ln_conflict_existing_item = 2
};

typedef int32_t LN_FAILURE;

enum
{
    ln_failure_none = 0,
    ln_failure_in_use = 1,
    ln_failure_access_denied = 2,
    ln_failure_not_found = 3,
    ln_failure_already_exists = 4,
    ln_failure_replaced = 5,
    ln_failure_path_too_long = 6,
    ln_failure_depends_on_failed = 7,
    ln_failure_other = 8
};

typedef int32_t LN_STAGE;

enum
{
    ln_stage_check = 0,
    ln_stage_run = 1,
    ln_stage_revert = 2
};

typedef int32_t LN_MOD_FIELD;

enum
{
    ln_mod_field_targets = 0,
    ln_mod_field_find_text = 1,
    ln_mod_field_start_text = 2,
    ln_mod_field_end_text = 3,
    ln_mod_field_value = 4,
    ln_mod_field_modified_date_format = 5,
    ln_mod_field_created_date_format = 6
};

typedef int32_t LN_PROBLEM_REASON;

enum
{
    ln_problem_reason_required = 0
};

typedef int32_t LN_INDEX_ERROR;

enum
{
    ln_index_error_none = 0,
    ln_index_error_syntax = 1,
    ln_index_error_out_of_range = 2
};

typedef int32_t LN_SORT_KEY;

enum
{
    ln_sort_key_added = 0,
    ln_sort_key_name_natural = 1,
    ln_sort_key_name_ordinal = 2,
    ln_sort_key_modified = 3,
    ln_sort_key_custom = 4
};

typedef int32_t LN_ADD_MODE;

enum
{
    ln_add_mode_as_items = 0,
    ln_add_mode_folder_contents = 1
};

typedef int32_t LN_RESOLVE_CHOICE;

enum
{
    ln_resolve_choice_revert_all = 0,
    ln_resolve_choice_keep_succeeded = 1
};

typedef int32_t LN_EXAMPLE_STATE;

enum
{
    ln_example_state_changed = 0,
    ln_example_state_unchanged = 1,
    ln_example_state_skipped_dir = 2,
    ln_example_state_no_target = 3
};

typedef struct LN_SESSION LN_SESSION;

typedef void (*LN_PROGRESS_FN)(void *user, uint32_t done, uint32_t total);

typedef struct LN_ROW
{
    uint64_t item_id;
    const char *folder;
    const char *name;
    const char *new_name;
    int64_t size;
    int64_t modified_ms;
    int64_t created_ms;
    uint32_t prefix_len;
    uint32_t suffix_len;
    uint32_t errors;
    uint32_t warnings;
    int32_t status;
    int32_t conflict;
    uint32_t conflict_index;
    uint8_t is_dir;
    uint8_t changed;
    uint8_t adjusted;
    uint8_t direct;
} LN_ROW;

typedef struct LN_PREVIEW
{
    const LN_ROW *rows;
    size_t count;
    uint32_t changed;
    uint32_t runnable;
    uint32_t warning;
    uint32_t error;
    uint32_t conflict;
} LN_PREVIEW;

typedef struct LN_EXAMPLE
{
    uint64_t item_id;
    uint32_t index;
    const char *before;
    const char *after;
    uint32_t prefix_len;
    uint32_t suffix_len;
    uint32_t errors;
    int32_t state;
} LN_EXAMPLE;

typedef struct LN_REPORT_ITEM
{
    uint64_t item_id;
    const char *from_path;
    const char *to_path;
    const char *current_path;
    int32_t stage;
    int32_t failure;
    int32_t os_error;
} LN_REPORT_ITEM;

typedef struct LN_REPORT
{
    const LN_REPORT_ITEM *problems;
    size_t problem_count;
    uint32_t total;
    uint32_t succeeded;
    uint32_t skipped;
    uint32_t failed;
    uint8_t needs_choice;
} LN_REPORT;

typedef struct LN_PROBLEM
{
    int32_t field;
    int32_t reason;
} LN_PROBLEM;

typedef struct LN_PROBLEMS
{
    const LN_PROBLEM *items;
    size_t count;
} LN_PROBLEMS;

typedef struct LN_INDEX_LIST
{
    const uint32_t *indices;
    size_t count;
    int32_t error;
} LN_INDEX_LIST;

typedef struct LN_ADD_RESULT
{
    uint32_t added;
    uint32_t skipped;
    uint32_t failed;
} LN_ADD_RESULT;

LN_API LN_RESULT ln_session_create(LN_SESSION **out);
LN_API void ln_session_destroy(LN_SESSION *session);
LN_API const char *ln_version(void);

LN_API LN_RESULT ln_items_add(LN_SESSION *session, const char *const *paths, size_t count,
    int32_t mode, LN_ADD_RESULT *out);
LN_API LN_RESULT ln_items_remove(LN_SESSION *session, const uint64_t *ids, size_t count);
LN_API LN_RESULT ln_items_clear(LN_SESSION *session);
LN_API LN_RESULT ln_items_move(LN_SESSION *session, const uint64_t *ids, size_t count,
    uint64_t target_id, uint8_t after, uint8_t dry_run, uint8_t *changed);
LN_API LN_RESULT ln_items_sort(LN_SESSION *session, int32_t key, uint8_t descending);
LN_API LN_RESULT ln_items_sort_state(LN_SESSION *session, int32_t *key, uint8_t *descending);
LN_API LN_RESULT ln_folders_refresh(LN_SESSION *session);

LN_API LN_RESULT ln_preview(LN_SESSION *session, LN_PREVIEW **out);

LN_API void ln_string_free(char *text);
LN_API void ln_problems_free(LN_PROBLEMS *problems);
LN_API void ln_example_free(LN_EXAMPLE *example);
LN_API void ln_preview_free(LN_PREVIEW *preview);
LN_API void ln_report_free(LN_REPORT *report);
LN_API void ln_index_list_free(LN_INDEX_LIST *list);

#ifdef __cplusplus
}
#endif
