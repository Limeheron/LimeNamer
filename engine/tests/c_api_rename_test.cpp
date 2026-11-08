#include "c_api_rename_test.h"

namespace
{
const char *const PREFIX =
    R"({"kind":"insert","position":{"type":"start"},"value":[{"text":"a_"}]})";

using CALLS = std::vector<std::pair<uint32_t, uint32_t>>;
using SYNC = std::pair<std::promise<void>, std::shared_future<void>>;

void record_progress(void *user, uint32_t done, uint32_t total)
{
    static_cast<CALLS *>(user)->emplace_back(done, total);
}

void wait_in_progress(void *user, uint32_t, uint32_t)
{
    SYNC &sync = *static_cast<SYNC *>(user);
    sync.first.set_value();
    sync.second.wait();
}

#if defined(_WIN32)
constexpr const int32_t LOCKED = ln_failure_in_use;
#else
constexpr const int32_t LOCKED = ln_failure_access_denied;
#endif
}

TEST_F(C_API_TEST, renames_and_undoes_with_progress)
{
    LN_ADD_RESULT added{};
    add({make_file("x.txt"), make_file("y.txt")}, ln_add_mode_as_items, added);
    uint64_t id = 0;
    ln_mod_add(session_, PREFIX, &id);
    CALLS calls;
    LN_REPORT *report = nullptr;
    ASSERT_EQ(ln_rename(session_, record_progress, &calls, &report), ln_result_ok);
    EXPECT_EQ(report->total, 2u);
    EXPECT_EQ(report->succeeded, 2u);
    EXPECT_EQ(report->needs_choice, 0);
    EXPECT_EQ(report->problem_count, size_t{0});
    ln_report_free(report);
    EXPECT_EQ(calls, CALLS({{1, 2}, {2, 2}}));
    EXPECT_EQ(names(), std::vector<std::string>({"a_x.txt", "a_y.txt"}));
    EXPECT_TRUE(std::filesystem::exists(root_ / "a_x.txt"));

    uint8_t available = 9;
    uint32_t count = 9;
    ASSERT_EQ(ln_rename_undo_state(session_, &available, &count), ln_result_ok);
    EXPECT_EQ(available, 1);
    EXPECT_EQ(count, 2u);
    ASSERT_EQ(ln_rename_undo(session_, nullptr, nullptr, &report), ln_result_ok);
    EXPECT_EQ(report->succeeded, 2u);
    ln_report_free(report);
    EXPECT_EQ(names(), std::vector<std::string>({"x.txt", "y.txt"}));
    EXPECT_TRUE(std::filesystem::exists(root_ / "x.txt"));
    ASSERT_EQ(ln_rename_undo_state(session_, &available, &count), ln_result_ok);
    EXPECT_EQ(available, 0);
    EXPECT_EQ(count, 0u);

    report = reinterpret_cast<LN_REPORT *>(&calls);
    EXPECT_EQ(ln_rename_undo(session_, nullptr, nullptr, &report), ln_result_not_ready);
    EXPECT_EQ(report, nullptr);
    EXPECT_EQ(ln_rename_undo_state(session_, nullptr, &count), ln_result_invalid_argument);
}

TEST_F(C_API_TEST, refuses_to_rename_with_conflicts)
{
    LN_ADD_RESULT added{};
    add({make_file("x.txt"), make_file("y.txt")}, ln_add_mode_as_items, added);
    LN_REPORT *report = nullptr;
    ASSERT_EQ(ln_rename(session_, nullptr, nullptr, &report), ln_result_ok);
    EXPECT_EQ(report->total, 0u);
    ln_report_free(report);

    uint64_t id = 0;
    ln_mod_add(session_,
        R"({"kind":"replace","position":{"type":"all"},"value":[{"text":"same"}]})", &id);
    EXPECT_EQ(ln_rename(session_, nullptr, nullptr, &report), ln_result_not_ready);
    EXPECT_EQ(report, nullptr);
    EXPECT_TRUE(std::filesystem::exists(root_ / "x.txt"));
    EXPECT_EQ(ln_rename(session_, nullptr, nullptr, nullptr), ln_result_invalid_argument);
    EXPECT_EQ(ln_rename(nullptr, nullptr, nullptr, &report), ln_result_invalid_argument);
}

TEST_F(C_API_TEST, waits_for_a_choice_after_a_failure)
{
    LN_ADD_RESULT added{};
    add({make_file("a.txt"), make_file("b.txt"), make_file("c.txt")}, ln_add_mode_as_items,
        added);
    const std::vector<uint64_t> items = ids();
    uint64_t id = 0;
    ln_mod_add(session_, PREFIX, &id);
    {
        const FILE_LOCK lock(root_ / "b.txt");
        LN_REPORT *report = nullptr;
        ASSERT_EQ(ln_rename(session_, nullptr, nullptr, &report), ln_result_ok);
        EXPECT_EQ(report->succeeded, 2u);
        EXPECT_EQ(report->failed, 1u);
        EXPECT_EQ(report->needs_choice, 1);
        ASSERT_EQ(report->problem_count, size_t{1});
        EXPECT_EQ(report->problems[0].item_id, items[1]);
        EXPECT_EQ(std::string(report->problems[0].current_path), utf8(root_ / "b.txt"));
        EXPECT_EQ(report->problems[0].stage, ln_stage_run);
        EXPECT_EQ(report->problems[0].failure, LOCKED);
        ln_report_free(report);

        EXPECT_EQ(ln_items_clear(session_), ln_result_busy);
        EXPECT_EQ(ln_mod_add(session_, PREFIX, &id), ln_result_busy);
        EXPECT_EQ(ln_rename(session_, nullptr, nullptr, &report), ln_result_busy);
        EXPECT_EQ(ln_rename_undo(session_, nullptr, nullptr, &report), ln_result_busy);
        EXPECT_EQ(names().size(), size_t{3});
        EXPECT_EQ(ln_rename_resolve(session_, 2, nullptr, nullptr, &report),
            ln_result_invalid_argument);
        ASSERT_EQ(ln_rename_resolve(session_, ln_resolve_choice_keep_succeeded, nullptr,
            nullptr, &report), ln_result_ok);
        EXPECT_EQ(report->succeeded, 2u);
        ln_report_free(report);
        EXPECT_EQ(ln_rename_resolve(session_, ln_resolve_choice_keep_succeeded, nullptr,
            nullptr, &report), ln_result_not_found);
        EXPECT_EQ(report, nullptr);
    }
    EXPECT_EQ(names(), std::vector<std::string>({"a_a.txt", "b.txt", "a_c.txt"}));
    LN_REPORT *report = nullptr;
    ASSERT_EQ(ln_rename_undo(session_, nullptr, nullptr, &report), ln_result_ok);
    EXPECT_EQ(report->succeeded, 2u);
    ln_report_free(report);
    EXPECT_EQ(names(), std::vector<std::string>({"a.txt", "b.txt", "c.txt"}));
}

TEST_F(C_API_TEST, reverts_all_on_choice)
{
    LN_ADD_RESULT added{};
    add({make_file("a.txt"), make_file("b.txt")}, ln_add_mode_as_items, added);
    uint64_t id = 0;
    ln_mod_add(session_, PREFIX, &id);
    const FILE_LOCK lock(root_ / "b.txt");
    LN_REPORT *report = nullptr;
    ASSERT_EQ(ln_rename(session_, nullptr, nullptr, &report), ln_result_ok);
    ASSERT_EQ(report->needs_choice, 1);
    ln_report_free(report);
    CALLS calls;
    ASSERT_EQ(ln_rename_resolve(session_, ln_resolve_choice_revert_all, record_progress, &calls,
        &report), ln_result_ok);
    ln_report_free(report);
    EXPECT_EQ(calls, CALLS({{1, 1}}));
    EXPECT_EQ(names(), std::vector<std::string>({"a.txt", "b.txt"}));
    EXPECT_TRUE(std::filesystem::exists(root_ / "a.txt"));
    uint8_t available = 9;
    uint32_t count = 9;
    ASSERT_EQ(ln_rename_undo_state(session_, &available, &count), ln_result_ok);
    EXPECT_EQ(available, 0);
}

TEST_F(C_API_TEST, rejects_calls_while_renaming)
{
    LN_ADD_RESULT added{};
    add({make_file("x.txt")}, ln_add_mode_as_items, added);
    uint64_t id = 0;
    ln_mod_add(session_, PREFIX, &id);
    std::promise<void> release;
    SYNC sync;
    sync.second = release.get_future().share();
    std::future<void> entered = sync.first.get_future();
    LN_REPORT *report = nullptr;
    LN_RESULT result = ln_result_internal;
    std::thread worker([&]() {
        result = ln_rename(session_, wait_in_progress, &sync, &report);
    });
    entered.wait();
    LN_PREVIEW *preview = nullptr;
    EXPECT_EQ(ln_preview(session_, &preview), ln_result_busy);
    EXPECT_EQ(preview, nullptr);
    EXPECT_EQ(ln_items_clear(session_), ln_result_busy);
    uint8_t available = 0;
    uint32_t count = 0;
    EXPECT_EQ(ln_rename_undo_state(session_, &available, &count), ln_result_busy);
    release.set_value();
    worker.join();
    ASSERT_EQ(result, ln_result_ok);
    EXPECT_EQ(report->succeeded, 1u);
    ln_report_free(report);
    EXPECT_EQ(names(), std::vector<std::string>({"a_x.txt"}));
}

TEST_F(C_API_TEST, reports_undo_checks_by_listed_items)
{
    LN_ADD_RESULT added{};
    add({make_file("x.txt"), make_file("y.txt")}, ln_add_mode_as_items, added);
    uint64_t id = 0;
    ln_mod_add(session_, PREFIX, &id);
    LN_REPORT *report = nullptr;
    ASSERT_EQ(ln_rename(session_, nullptr, nullptr, &report), ln_result_ok);
    ln_report_free(report);
    const std::vector<uint64_t> items = ids();
    ln_items_remove(session_, &items[0], 1);
    std::filesystem::remove(root_ / "a_x.txt");

    ASSERT_EQ(ln_rename_undo_check(session_, &report), ln_result_ok);
    EXPECT_EQ(report->total, 2u);
    EXPECT_EQ(report->skipped, 1u);
    ASSERT_EQ(report->problem_count, size_t{1});
    EXPECT_EQ(report->problems[0].item_id, 0u);
    EXPECT_EQ(report->problems[0].stage, ln_stage_check);
    EXPECT_EQ(report->problems[0].failure, ln_failure_not_found);
    EXPECT_EQ(std::string(report->problems[0].from_path), utf8(root_ / "a_x.txt"));
    EXPECT_EQ(std::string(report->problems[0].to_path), utf8(root_ / "x.txt"));
    ln_report_free(report);

    ASSERT_EQ(ln_rename_undo(session_, nullptr, nullptr, &report), ln_result_ok);
    EXPECT_EQ(report->succeeded, 1u);
    EXPECT_EQ(report->skipped, 1u);
    ln_report_free(report);
    EXPECT_EQ(names(), std::vector<std::string>({"y.txt"}));
    uint8_t available = 0;
    uint32_t count = 0;
    ASSERT_EQ(ln_rename_undo_state(session_, &available, &count), ln_result_ok);
    EXPECT_EQ(count, 1u);
    EXPECT_EQ(ln_rename_undo_check(session_, nullptr), ln_result_invalid_argument);
}
