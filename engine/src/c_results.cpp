#include "c_results.h"

static_assert(static_cast<int32_t>(ROW_STATUS::ok) == ln_status_ok);
static_assert(static_cast<int32_t>(ROW_STATUS::unchanged) == ln_status_unchanged);
static_assert(static_cast<int32_t>(ROW_STATUS::warning) == ln_status_warning);
static_assert(static_cast<int32_t>(ROW_STATUS::error) == ln_status_error);
static_assert(static_cast<int32_t>(ROW_STATUS::conflict) == ln_status_conflict);

static_assert(NAME_ISSUE::EMPTY == static_cast<uint32_t>(ln_name_issue_empty));
static_assert(NAME_ISSUE::DOT_NAME == static_cast<uint32_t>(ln_name_issue_dot_name));
static_assert(NAME_ISSUE::WIN_FORBIDDEN_CHAR
    == static_cast<uint32_t>(ln_name_issue_win_forbidden_char));
static_assert(NAME_ISSUE::WIN_RESERVED == static_cast<uint32_t>(ln_name_issue_win_reserved));
static_assert(NAME_ISSUE::WIN_TRAILING == static_cast<uint32_t>(ln_name_issue_win_trailing));
static_assert(NAME_ISSUE::WIN_NAME_TOO_LONG
    == static_cast<uint32_t>(ln_name_issue_win_name_too_long));
static_assert(NAME_ISSUE::WIN_PATH_TOO_LONG
    == static_cast<uint32_t>(ln_name_issue_win_path_too_long));
static_assert(NAME_ISSUE::MAC_FORBIDDEN_CHAR
    == static_cast<uint32_t>(ln_name_issue_mac_forbidden_char));
static_assert(NAME_ISSUE::MAC_NAME_TOO_LONG
    == static_cast<uint32_t>(ln_name_issue_mac_name_too_long));
static_assert(NAME_ISSUE::HIDDEN == static_cast<uint32_t>(ln_name_issue_hidden));

static_assert(static_cast<int32_t>(CONFLICT_KIND::none) == ln_conflict_none);
static_assert(static_cast<int32_t>(CONFLICT_KIND::list_item) == ln_conflict_list_item);
static_assert(static_cast<int32_t>(CONFLICT_KIND::existing_item) == ln_conflict_existing_item);

static_assert(static_cast<int32_t>(RENAME_FAILURE::none) == ln_failure_none);
static_assert(static_cast<int32_t>(RENAME_FAILURE::in_use) == ln_failure_in_use);
static_assert(static_cast<int32_t>(RENAME_FAILURE::access_denied) == ln_failure_access_denied);
static_assert(static_cast<int32_t>(RENAME_FAILURE::not_found) == ln_failure_not_found);
static_assert(static_cast<int32_t>(RENAME_FAILURE::already_exists)
    == ln_failure_already_exists);
static_assert(static_cast<int32_t>(RENAME_FAILURE::replaced) == ln_failure_replaced);
static_assert(static_cast<int32_t>(RENAME_FAILURE::path_too_long) == ln_failure_path_too_long);
static_assert(static_cast<int32_t>(RENAME_FAILURE::depends_on_failed)
    == ln_failure_depends_on_failed);
static_assert(static_cast<int32_t>(RENAME_FAILURE::other) == ln_failure_other);

static_assert(static_cast<int32_t>(REPORT_STAGE::check) == ln_stage_check);
static_assert(static_cast<int32_t>(REPORT_STAGE::run) == ln_stage_run);
static_assert(static_cast<int32_t>(REPORT_STAGE::revert) == ln_stage_revert);

static_assert(static_cast<int32_t>(MOD_FIELD::targets) == ln_mod_field_targets);
static_assert(static_cast<int32_t>(MOD_FIELD::find_text) == ln_mod_field_find_text);
static_assert(static_cast<int32_t>(MOD_FIELD::start_text) == ln_mod_field_start_text);
static_assert(static_cast<int32_t>(MOD_FIELD::end_text) == ln_mod_field_end_text);
static_assert(static_cast<int32_t>(MOD_FIELD::value) == ln_mod_field_value);
static_assert(static_cast<int32_t>(MOD_FIELD::modified_date_format)
    == ln_mod_field_modified_date_format);
static_assert(static_cast<int32_t>(MOD_FIELD::created_date_format)
    == ln_mod_field_created_date_format);
static_assert(static_cast<int32_t>(PROBLEM_REASON::required) == ln_problem_reason_required);

static_assert(static_cast<int32_t>(INDEX_ERROR::none) == ln_index_error_none);
static_assert(static_cast<int32_t>(INDEX_ERROR::syntax) == ln_index_error_syntax);
static_assert(static_cast<int32_t>(INDEX_ERROR::out_of_range) == ln_index_error_out_of_range);

static_assert(static_cast<int32_t>(EXAMPLE_STATE::changed) == ln_example_state_changed);
static_assert(static_cast<int32_t>(EXAMPLE_STATE::unchanged) == ln_example_state_unchanged);
static_assert(static_cast<int32_t>(EXAMPLE_STATE::skipped_dir)
    == ln_example_state_skipped_dir);
static_assert(static_cast<int32_t>(EXAMPLE_STATE::no_target) == ln_example_state_no_target);

LN_PREVIEW *C_RESULTS::preview(const PREVIEW &preview)
{
    const size_t count = preview.rows.size();
    std::vector<std::string> folders;
    std::vector<size_t> folder_of(count);
    std::unordered_map<std::filesystem::path::string_type, size_t> folder_index;
    size_t text_size = 0;
    for (size_t index = 0; index < count; ++index) {
        const PREVIEW_ROW &row = preview.rows[index];
        const auto [found, added] = folder_index.try_emplace(row.folder.native(), folders.size());
        if (added) {
            folders.push_back(utf8_of(row.folder));
            text_size += folders.back().size() + 1;
        }
        folder_of[index] = found->second;
        text_size += UTF::utf8_length(row.name) + 1 + UTF::utf8_length(row.new_name) + 1;
    }

    C_BLOCK<LN_PREVIEW, LN_ROW> block(count, text_size);
    std::vector<const char *> folder_texts;
    folder_texts.reserve(folders.size());
    for (const std::string &folder : folders) {
        folder_texts.push_back(block.copy(folder));
    }
    LN_PREVIEW &head = block.head();
    head.rows = block.items();
    head.count = count;
    head.changed = preview.changed;
    head.runnable = preview.runnable;
    head.warning = preview.warning;
    head.error = preview.error;
    head.conflict = preview.conflict;
    for (size_t index = 0; index < count; ++index) {
        const PREVIEW_ROW &row = preview.rows[index];
        LN_ROW &out = block.items()[index];
        out.item_id = row.item_id;
        out.folder = folder_texts[folder_of[index]];
        out.name = block.copy(UTF::encode_utf8(row.name));
        out.new_name = block.copy(UTF::encode_utf8(row.new_name));
        out.size = row.size;
        out.modified_ms = row.modified_ms;
        out.created_ms = row.created_ms;
        out.prefix_len = row.prefix_len;
        out.suffix_len = row.suffix_len;
        out.errors = row.errors;
        out.warnings = row.warnings;
        out.status = static_cast<int32_t>(row.status);
        out.conflict = static_cast<int32_t>(row.conflict);
        out.conflict_index = row.conflict_index;
        out.is_dir = row.is_dir ? 1 : 0;
        out.changed = row.changed ? 1 : 0;
        out.adjusted = row.adjusted ? 1 : 0;
        out.direct = row.direct ? 1 : 0;
    }
    return block.release();
}

LN_EXAMPLE *C_RESULTS::example(const EXAMPLE &example)
{
    const std::string before = UTF::encode_utf8(example.before);
    const std::string after = UTF::encode_utf8(example.after);
    C_BLOCK<LN_EXAMPLE, char> block(0, before.size() + 1 + after.size() + 1);
    LN_EXAMPLE &head = block.head();
    head.item_id = example.item_id;
    head.index = example.index;
    head.before = block.copy(before);
    head.after = block.copy(after);
    head.prefix_len = example.prefix_len;
    head.suffix_len = example.suffix_len;
    head.errors = example.errors;
    head.state = static_cast<int32_t>(example.state);
    return block.release();
}

LN_REPORT *C_RESULTS::report(const RENAME_REPORT &report)
{
    const size_t count = report.problems.size();
    std::vector<std::string> texts;
    texts.reserve(count * 3);
    size_t text_size = 0;
    for (const REPORT_ITEM &item : report.problems) {
        for (const std::filesystem::path *path : {&item.from, &item.to, &item.current}) {
            texts.push_back(utf8_of(*path));
            text_size += texts.back().size() + 1;
        }
    }

    C_BLOCK<LN_REPORT, LN_REPORT_ITEM> block(count, text_size);
    LN_REPORT &head = block.head();
    head.problems = block.items();
    head.problem_count = count;
    head.total = report.total;
    head.succeeded = report.succeeded;
    head.skipped = report.skipped;
    head.failed = report.failed;
    head.needs_choice = report.needs_choice ? 1 : 0;
    for (size_t index = 0; index < count; ++index) {
        const REPORT_ITEM &item = report.problems[index];
        LN_REPORT_ITEM &out = block.items()[index];
        out.item_id = item.item_id;
        out.from_path = block.copy(texts[index * 3]);
        out.to_path = block.copy(texts[index * 3 + 1]);
        out.current_path = block.copy(texts[index * 3 + 2]);
        out.stage = static_cast<int32_t>(item.stage);
        out.failure = static_cast<int32_t>(item.failure);
        out.os_error = item.os_error;
    }
    return block.release();
}

LN_PROBLEMS *C_RESULTS::problems(const std::vector<MOD_PROBLEM> &problems)
{
    C_BLOCK<LN_PROBLEMS, LN_PROBLEM> block(problems.size(), 0);
    LN_PROBLEMS &head = block.head();
    head.items = block.items();
    head.count = problems.size();
    for (size_t index = 0; index < problems.size(); ++index) {
        block.items()[index].field = static_cast<int32_t>(problems[index].field);
        block.items()[index].reason = static_cast<int32_t>(problems[index].reason);
    }
    return block.release();
}

LN_INDEX_LIST *C_RESULTS::index_list(const INDEX_LIST_RESULT &result)
{
    C_BLOCK<LN_INDEX_LIST, uint32_t> block(result.indices.size(), 0);
    LN_INDEX_LIST &head = block.head();
    head.indices = block.items();
    head.count = result.indices.size();
    head.error = static_cast<int32_t>(result.error);
    if (!result.indices.empty()) {
        std::memcpy(block.items(), result.indices.data(),
            result.indices.size() * sizeof(uint32_t));
    }
    return block.release();
}

char *C_RESULTS::string(std::string_view text)
{
    char *result = static_cast<char *>(std::malloc(text.size() + 1));
    if (result == nullptr) {
        throw std::bad_alloc();
    }
    if (!text.empty()) {
        std::memcpy(result, text.data(), text.size());
    }
    result[text.size()] = '\0';
    return result;
}

LN_ADD_RESULT C_RESULTS::add_result(const ADD_RESULT &result)
{
    LN_ADD_RESULT out;
    out.added = result.added;
    out.skipped = result.skipped;
    out.failed = result.failed;
    return out;
}

void C_RESULTS::release(void *result)
{
    std::free(result);
}

std::string C_RESULTS::utf8_of(const std::filesystem::path &path)
{
    const std::optional<std::string> text = UTF::path_to_utf8(path);
    if (!text) {
        throw std::logic_error("path is not valid unicode");
    }
    return *text;
}
