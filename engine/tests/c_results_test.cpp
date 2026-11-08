#include "c_results_test.h"

TEST_F(C_RESULTS_TEST, converts_previews)
{
    PREVIEW preview;
    preview.rows.push_back(row(4, U"/폴더", U"a.txt", U"가.txt"));
    preview.rows.push_back(row(2, U"/other", U"b", U"b"));
    preview.rows.push_back(row(7, U"/폴더", U"c.txt", U"d.txt"));
    PREVIEW_ROW &first = preview.rows[0];
    first.size = 2048;
    first.modified_ms = -5;
    first.created_ms = 1700000000000;
    first.prefix_len = 1;
    first.suffix_len = 4;
    first.errors = NAME_ISSUE::WIN_RESERVED;
    first.warnings = NAME_ISSUE::HIDDEN;
    first.status = ROW_STATUS::conflict;
    first.conflict = CONFLICT_KIND::list_item;
    first.conflict_index = 3;
    first.changed = true;
    first.adjusted = true;
    first.direct = true;
    preview.rows[1].is_dir = true;
    preview.changed = 2;
    preview.runnable = 1;
    preview.warning = 3;
    preview.error = 4;
    preview.conflict = 5;

    const OWNED<LN_PREVIEW> result = owned(C_RESULTS::preview(preview));
    ASSERT_EQ(result->count, size_t{3});
    EXPECT_EQ(result->changed, 2u);
    EXPECT_EQ(result->runnable, 1u);
    EXPECT_EQ(result->warning, 3u);
    EXPECT_EQ(result->error, 4u);
    EXPECT_EQ(result->conflict, 5u);
    const LN_ROW &out = result->rows[0];
    EXPECT_EQ(out.item_id, 4u);
    EXPECT_EQ(std::string(out.folder), "/\xed\x8f\xb4\xeb\x8d\x94");
    EXPECT_EQ(std::string(out.name), "a.txt");
    EXPECT_EQ(std::string(out.new_name), "\xea\xb0\x80.txt");
    EXPECT_EQ(out.size, 2048);
    EXPECT_EQ(out.modified_ms, -5);
    EXPECT_EQ(out.created_ms, 1700000000000);
    EXPECT_EQ(out.prefix_len, 1u);
    EXPECT_EQ(out.suffix_len, 4u);
    EXPECT_EQ(out.errors, static_cast<uint32_t>(ln_name_issue_win_reserved));
    EXPECT_EQ(out.warnings, static_cast<uint32_t>(ln_name_issue_hidden));
    EXPECT_EQ(out.status, ln_status_conflict);
    EXPECT_EQ(out.conflict, ln_conflict_list_item);
    EXPECT_EQ(out.conflict_index, 3u);
    EXPECT_EQ(out.is_dir, 0);
    EXPECT_EQ(out.changed, 1);
    EXPECT_EQ(out.adjusted, 1);
    EXPECT_EQ(out.direct, 1);
    EXPECT_EQ(result->rows[1].is_dir, 1);
    EXPECT_EQ(result->rows[1].status, ln_status_unchanged);
    EXPECT_EQ(std::string(result->rows[1].folder), "/other");
    EXPECT_EQ(result->rows[2].folder, out.folder);
    EXPECT_EQ(std::string(result->rows[2].new_name), "d.txt");
}

TEST_F(C_RESULTS_TEST, converts_empty_previews)
{
    const OWNED<LN_PREVIEW> result = owned(C_RESULTS::preview(PREVIEW()));
    EXPECT_EQ(result->rows, nullptr);
    EXPECT_EQ(result->count, size_t{0});
}

TEST_F(C_RESULTS_TEST, converts_examples)
{
    EXAMPLE example;
    example.item_id = 9;
    example.index = 2;
    example.before = U"IMG_0001.JPG";
    example.after = U"제주_0001.JPG";
    example.prefix_len = 0;
    example.suffix_len = 8;
    example.errors = NAME_ISSUE::EMPTY;
    example.state = EXAMPLE_STATE::changed;
    const OWNED<LN_EXAMPLE> result = owned(C_RESULTS::example(example));
    EXPECT_EQ(result->item_id, 9u);
    EXPECT_EQ(result->index, 2u);
    EXPECT_EQ(std::string(result->before), "IMG_0001.JPG");
    EXPECT_EQ(std::string(result->after), "\xec\xa0\x9c\xec\xa3\xbc_0001.JPG");
    EXPECT_EQ(result->suffix_len, 8u);
    EXPECT_EQ(result->errors, static_cast<uint32_t>(ln_name_issue_empty));
    EXPECT_EQ(result->state, ln_example_state_changed);
}

TEST_F(C_RESULTS_TEST, converts_reports)
{
    RENAME_REPORT report;
    report.total = 5;
    report.succeeded = 2;
    report.skipped = 1;
    report.failed = 2;
    report.needs_choice = true;
    report.problems.push_back({3, "/a/x.txt", "/a/y.txt", "/a/x.txt", REPORT_STAGE::run,
        RENAME_FAILURE::in_use, 32});
    report.problems.push_back({0, "/a/b.txt", "/a/c.txt", "/a/.limenamer-tmp-1",
        REPORT_STAGE::revert, RENAME_FAILURE::access_denied, 5});
    const OWNED<LN_REPORT> result = owned(C_RESULTS::report(report));
    EXPECT_EQ(result->total, 5u);
    EXPECT_EQ(result->succeeded, 2u);
    EXPECT_EQ(result->skipped, 1u);
    EXPECT_EQ(result->failed, 2u);
    EXPECT_EQ(result->needs_choice, 1);
    ASSERT_EQ(result->problem_count, size_t{2});
    const LN_REPORT_ITEM &first = result->problems[0];
    EXPECT_EQ(first.item_id, 3u);
    EXPECT_EQ(std::string(first.from_path), "/a/x.txt");
    EXPECT_EQ(std::string(first.to_path), "/a/y.txt");
    EXPECT_EQ(std::string(first.current_path), "/a/x.txt");
    EXPECT_EQ(first.stage, ln_stage_run);
    EXPECT_EQ(first.failure, ln_failure_in_use);
    EXPECT_EQ(first.os_error, 32);
    const LN_REPORT_ITEM &second = result->problems[1];
    EXPECT_EQ(second.item_id, 0u);
    EXPECT_EQ(std::string(second.current_path), "/a/.limenamer-tmp-1");
    EXPECT_EQ(second.stage, ln_stage_revert);
    EXPECT_EQ(second.failure, ln_failure_access_denied);

    const OWNED<LN_REPORT> empty = owned(C_RESULTS::report(RENAME_REPORT()));
    EXPECT_EQ(empty->problems, nullptr);
    EXPECT_EQ(empty->needs_choice, 0);
}

TEST_F(C_RESULTS_TEST, converts_problems)
{
    const std::vector<MOD_PROBLEM> problems = {
        {MOD_FIELD::targets, PROBLEM_REASON::required},
        {MOD_FIELD::created_date_format, PROBLEM_REASON::required}};
    const OWNED<LN_PROBLEMS> result = owned(C_RESULTS::problems(problems));
    ASSERT_EQ(result->count, size_t{2});
    EXPECT_EQ(result->items[0].field, ln_mod_field_targets);
    EXPECT_EQ(result->items[1].field, ln_mod_field_created_date_format);
    EXPECT_EQ(result->items[1].reason, ln_problem_reason_required);
}

TEST_F(C_RESULTS_TEST, converts_index_lists)
{
    INDEX_LIST_RESULT list;
    list.indices = {1, 2, 9};
    const OWNED<LN_INDEX_LIST> result = owned(C_RESULTS::index_list(list));
    ASSERT_EQ(result->count, size_t{3});
    EXPECT_EQ(result->indices[2], 9u);
    EXPECT_EQ(result->error, ln_index_error_none);

    INDEX_LIST_RESULT broken;
    broken.error = INDEX_ERROR::syntax;
    const OWNED<LN_INDEX_LIST> failed = owned(C_RESULTS::index_list(broken));
    EXPECT_EQ(failed->indices, nullptr);
    EXPECT_EQ(failed->error, ln_index_error_syntax);
}

TEST_F(C_RESULTS_TEST, converts_strings_and_counts)
{
    const OWNED<char> text = owned(C_RESULTS::string("[{\"id\":1}]"));
    EXPECT_EQ(std::string(text.get()), "[{\"id\":1}]");
    const OWNED<char> empty = owned(C_RESULTS::string(""));
    EXPECT_EQ(std::string(empty.get()), "");

    const LN_ADD_RESULT added = C_RESULTS::add_result({3, 2, 1});
    EXPECT_EQ(added.added, 3u);
    EXPECT_EQ(added.skipped, 2u);
    EXPECT_EQ(added.failed, 1u);
}
