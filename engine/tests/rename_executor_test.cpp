#include "rename_executor_test.h"

TEST_F(RENAME_EXECUTOR_TEST, renames_everything)
{
    const RENAME_REPORT report = run({request(1, "a.txt", U"x.txt"),
        request(2, "b.txt", U"a.txt")});
    EXPECT_EQ(report.total, 2u);
    EXPECT_EQ(report.succeeded, 2u);
    EXPECT_EQ(report.failed, 0u);
    EXPECT_TRUE(report.problems.empty());
    EXPECT_FALSE(report.needs_choice);
    EXPECT_FALSE(executor_.pending());
    EXPECT_EQ(names(), std::vector<std::filesystem::path>({"a.txt", "x.txt"}));
    EXPECT_EQ(track(1).current, env_.root() / "x.txt");
}

TEST_F(RENAME_EXECUTOR_TEST, reports_progress_for_each_step)
{
    std::vector<uint32_t> seen;
    uint32_t last_total = 0;
    const std::vector<RENAME_REQUEST> requests = {request(1, "a", U"b"), request(2, "b", U"a")};
    FOLDER_CACHE cache(env_.file_system(), env_.text_service());
    cache.load(env_.root());
    executor_.run(requests, planner_.plan(requests, cache), [&](uint32_t done, uint32_t total) {
        seen.push_back(done);
        last_total = total;
    });
    EXPECT_EQ(seen, std::vector<uint32_t>({1, 2, 3}));
    EXPECT_EQ(last_total, 3u);
}

TEST_F(RENAME_EXECUTOR_TEST, continues_after_a_failure)
{
    fail("b.txt", FS_ERROR::in_use);
    const RENAME_REPORT report = run({request(1, "a.txt", U"x.txt"),
        request(2, "b.txt", U"y.txt"), request(3, "c.txt", U"z.txt")});
    EXPECT_EQ(report.succeeded, 2u);
    EXPECT_EQ(report.failed, 1u);
    ASSERT_EQ(report.problems.size(), size_t{1});
    EXPECT_EQ(report.problems[0].item_id, 2u);
    EXPECT_EQ(report.problems[0].stage, REPORT_STAGE::run);
    EXPECT_EQ(report.problems[0].failure, RENAME_FAILURE::in_use);
    EXPECT_EQ(report.problems[0].current, env_.root() / "b.txt");
    EXPECT_EQ(report.problems[0].to, env_.root() / "y.txt");
    EXPECT_TRUE(report.needs_choice);
    EXPECT_EQ(names(), std::vector<std::filesystem::path>({"b.txt", "x.txt", "z.txt"}));
}

TEST_F(RENAME_EXECUTOR_TEST, fails_items_waiting_for_a_failed_name)
{
    fail("b.txt", FS_ERROR::access_denied);
    const RENAME_REPORT report = run({request(1, "a.txt", U"b.txt"),
        request(2, "b.txt", U"c.txt")});
    EXPECT_EQ(report.failed, 2u);
    ASSERT_EQ(report.problems.size(), size_t{2});
    EXPECT_EQ(report.problems[0].item_id, 1u);
    EXPECT_EQ(report.problems[0].failure, RENAME_FAILURE::depends_on_failed);
    EXPECT_EQ(report.problems[1].failure, RENAME_FAILURE::access_denied);
    EXPECT_FALSE(report.needs_choice);
    EXPECT_EQ(names(), std::vector<std::filesystem::path>({"a.txt", "b.txt"}));
}

TEST_F(RENAME_EXECUTOR_TEST, restores_items_left_in_a_temporary_name)
{
    fail("b.txt", FS_ERROR::in_use);
    const RENAME_REPORT report = run({request(1, "a.txt", U"b.txt"),
        request(2, "b.txt", U"a.txt")});
    EXPECT_EQ(report.failed, 2u);
    EXPECT_EQ(report.succeeded, 0u);
    EXPECT_EQ(track(1).current, env_.root() / "a.txt");
    EXPECT_EQ(names(), std::vector<std::filesystem::path>({"a.txt", "b.txt"}));
    for (const REPORT_ITEM &problem : report.problems) {
        EXPECT_EQ(problem.stage, REPORT_STAGE::run);
    }
}

TEST_F(RENAME_EXECUTOR_TEST, reports_items_that_cannot_leave_a_temporary_name)
{
    fail(".limenamer-tmp-1", FS_ERROR::in_use);
    fail(".limenamer-tmp-1", FS_ERROR::in_use);
    const RENAME_REPORT report = run({request(1, "photo.jpg", U"Photo.jpg")});
    ASSERT_EQ(report.problems.size(), size_t{2});
    EXPECT_EQ(report.problems[0].stage, REPORT_STAGE::run);
    EXPECT_EQ(report.problems[1].stage, REPORT_STAGE::revert);
    EXPECT_EQ(report.problems[1].current, env_.root() / ".limenamer-tmp-1");
}

TEST_F(RENAME_EXECUTOR_TEST, keeps_successful_items_when_asked)
{
    fail("b.txt", FS_ERROR::in_use);
    run({request(1, "a.txt", U"x.txt"), request(2, "b.txt", U"y.txt")});
    const RENAME_REPORT report = executor_.resolve(RESOLVE_CHOICE::keep_succeeded, {});
    EXPECT_FALSE(report.needs_choice);
    EXPECT_FALSE(executor_.pending());
    EXPECT_EQ(report.succeeded, 1u);
    EXPECT_EQ(names(), std::vector<std::filesystem::path>({"b.txt", "x.txt"}));
    EXPECT_THROW(executor_.resolve(RESOLVE_CHOICE::keep_succeeded, {}), std::logic_error);
}

TEST_F(RENAME_EXECUTOR_TEST, reverts_everything_when_asked)
{
    fail("c.txt", FS_ERROR::in_use);
    run({request(1, "a.txt", U"b.txt"), request(2, "b.txt", U"a.txt"),
        request(3, "c.txt", U"d.txt")});
    const RENAME_REPORT report = executor_.resolve(RESOLVE_CHOICE::revert_all, {});
    EXPECT_EQ(report.succeeded, 0u);
    EXPECT_EQ(report.failed, 1u);
    ASSERT_EQ(report.problems.size(), size_t{1});
    EXPECT_EQ(names(), std::vector<std::filesystem::path>({"a.txt", "b.txt", "c.txt"}));
    EXPECT_EQ(track(1).current, env_.root() / "a.txt");
    EXPECT_TRUE(track(1).reverted);
}

TEST_F(RENAME_EXECUTOR_TEST, reverts_folders_and_their_items)
{
    fail("other.txt", FS_ERROR::in_use);
    const RENAME_REQUEST folder = request(1, "f", U"g", true);
    const RENAME_REQUEST file = request(2, "f/x.txt", U"y.txt");
    const RENAME_REQUEST other = request(3, "other.txt", U"new.txt");
    const RENAME_REPORT first = run({folder, file, other});
    EXPECT_EQ(first.succeeded, 2u);
    EXPECT_EQ(track(2).current, env_.root() / "g" / "y.txt");
    executor_.resolve(RESOLVE_CHOICE::revert_all, {});
    EXPECT_EQ(names("f"), std::vector<std::filesystem::path>({"x.txt"}));
    EXPECT_EQ(track(2).current, env_.root() / "f" / "x.txt");
}

TEST_F(RENAME_EXECUTOR_TEST, reports_items_that_cannot_be_reverted)
{
    fail("c.txt", FS_ERROR::in_use);
    run({request(1, "a.txt", U"x.txt"), request(2, "c.txt", U"d.txt")});
    fail("x.txt", FS_ERROR::access_denied);
    const RENAME_REPORT report = executor_.resolve(RESOLVE_CHOICE::revert_all, {});
    ASSERT_EQ(report.problems.size(), size_t{2});
    EXPECT_EQ(report.problems[1].item_id, 1u);
    EXPECT_EQ(report.problems[1].stage, REPORT_STAGE::revert);
    EXPECT_EQ(report.problems[1].failure, RENAME_FAILURE::access_denied);
    EXPECT_EQ(report.problems[1].current, env_.root() / "x.txt");
    EXPECT_EQ(report.succeeded, 1u);
}

TEST_F(RENAME_EXECUTOR_TEST, moves_tracked_items_with_their_folder)
{
    const RENAME_REQUEST folder = request(1, "f", U"g", true);
    const RENAME_REQUEST file = request(2, "f/x.txt", U"y.txt");
    run({file, folder});
    EXPECT_EQ(track(2).current, env_.root() / "g" / "y.txt");
    EXPECT_EQ(track(1).current, env_.root() / "g");
}

TEST_F(RENAME_EXECUTOR_TEST, maps_file_system_errors)
{
    EXPECT_EQ(RENAME_EXECUTOR::failure_of(FS_ERROR::not_found), RENAME_FAILURE::not_found);
    EXPECT_EQ(RENAME_EXECUTOR::failure_of(FS_ERROR::already_exists),
        RENAME_FAILURE::already_exists);
    EXPECT_EQ(RENAME_EXECUTOR::failure_of(FS_ERROR::path_too_long),
        RENAME_FAILURE::path_too_long);
    EXPECT_EQ(RENAME_EXECUTOR::failure_of(FS_ERROR::other), RENAME_FAILURE::other);
    EXPECT_TRUE(PATHS::is_under(std::filesystem::path("a") / "b",
        std::filesystem::path("a")));
    EXPECT_FALSE(PATHS::is_under(std::filesystem::path("ab"),
        std::filesystem::path("a")));
    EXPECT_FALSE(PATHS::is_under(std::filesystem::path("a"),
        std::filesystem::path("a")));
}
