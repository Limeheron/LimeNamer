#include "rename_planner_test.h"

TEST_F(RENAME_PLANNER_TEST, renames_independent_items_directly)
{
    const std::vector<RENAME_STEP> steps =
        plan({request(1, "a.txt", U"x.txt"), request(2, "b.txt", U"y.txt")});
    EXPECT_EQ(kinds(steps), std::vector<STEP_KIND>({STEP_KIND::direct, STEP_KIND::direct}));
    EXPECT_EQ(steps[0].item_id, 1u);
    EXPECT_EQ(steps[0].from, env_.root() / "a.txt");
    EXPECT_EQ(steps[0].to, env_.root() / "x.txt");
    execute(steps);
    EXPECT_EQ(names(), std::vector<std::filesystem::path>({"x.txt", "y.txt"}));
}

TEST_F(RENAME_PLANNER_TEST, orders_chains)
{
    const std::vector<RENAME_STEP> steps =
        plan({request(1, "a.txt", U"b.txt"), request(2, "b.txt", U"c.txt")});
    EXPECT_EQ(kinds(steps), std::vector<STEP_KIND>({STEP_KIND::direct, STEP_KIND::direct}));
    EXPECT_EQ(steps[0].item_id, 2u);
    execute(steps);
    EXPECT_EQ(names(), std::vector<std::filesystem::path>({"b.txt", "c.txt"}));
}

TEST_F(RENAME_PLANNER_TEST, orders_long_chains)
{
    std::vector<RENAME_REQUEST> requests;
    for (int number = 1; number <= 1000; ++number) {
        requests.push_back(request(static_cast<uint64_t>(number),
            std::to_string(number) + ".txt",
            UTF::path_to_u32(std::to_string(number + 1) + ".txt").value()));
    }
    const std::vector<RENAME_STEP> steps = plan(requests);
    EXPECT_EQ(steps.size(), size_t{1000});
    execute(steps);
    EXPECT_EQ(names().size(), size_t{1000});
    FILE_INFO info;
    EXPECT_FALSE(env_.file_system().get_info(env_.root() / "1.txt", info).ok());
    EXPECT_TRUE(env_.file_system().get_info(env_.root() / "1001.txt", info).ok());
}

TEST_F(RENAME_PLANNER_TEST, swaps_through_a_temporary_name)
{
    const std::vector<RENAME_STEP> steps =
        plan({request(1, "a.txt", U"b.txt"), request(2, "b.txt", U"a.txt")});
    EXPECT_EQ(kinds(steps), std::vector<STEP_KIND>(
        {STEP_KIND::to_temp, STEP_KIND::direct, STEP_KIND::from_temp}));
    EXPECT_EQ(steps[0].to.filename(), ".limenamer-tmp-1");
    execute(steps);
    EXPECT_EQ(names(), std::vector<std::filesystem::path>({"a.txt", "b.txt"}));
}

TEST_F(RENAME_PLANNER_TEST, breaks_cycles_with_one_temporary_name)
{
    const std::vector<RENAME_STEP> steps = plan({request(1, "a", U"b"), request(2, "b", U"c"),
        request(3, "c", U"a")});
    EXPECT_EQ(steps.size(), size_t{4});
    EXPECT_EQ(steps[0].kind, STEP_KIND::to_temp);
    execute(steps);
    EXPECT_EQ(names(), std::vector<std::filesystem::path>({"a", "b", "c"}));
}

TEST_F(RENAME_PLANNER_TEST, changes_case_through_a_temporary_name)
{
    const std::vector<RENAME_STEP> steps = plan({request(1, "photo.jpg", U"Photo.JPG")});
    EXPECT_EQ(kinds(steps), std::vector<STEP_KIND>({STEP_KIND::to_temp, STEP_KIND::from_temp}));
    execute(steps);
    EXPECT_EQ(names(), std::vector<std::filesystem::path>({"Photo.JPG"}));
}

TEST_F(RENAME_PLANNER_TEST, avoids_names_already_in_use)
{
    env_.make_file(env_.root() / ".limenamer-tmp-1", 1);
    const std::vector<RENAME_STEP> steps =
        plan({request(1, "a.txt", U"b.txt"), request(2, "b.txt", U"a.txt")});
    EXPECT_EQ(steps[0].to.filename(), ".limenamer-tmp-2");
    execute(steps);
}

TEST_F(RENAME_PLANNER_TEST, renames_inner_items_first)
{
    const RENAME_REQUEST folder = request(1, "f", U"g", true);
    const RENAME_REQUEST sub = request(2, "f/sub", U"inner", true);
    const RENAME_REQUEST file = request(3, "f/sub/x.txt", U"y.txt");
    const RENAME_REQUEST top = request(4, "f/z.txt", U"w.txt");
    const std::vector<RENAME_STEP> steps = plan({folder, sub, file, top});
    ASSERT_EQ(steps.size(), size_t{4});
    EXPECT_EQ(steps[0].item_id, 3u);
    EXPECT_EQ(steps[3].item_id, 1u);
    execute(steps);
    EXPECT_EQ(names("g"), std::vector<std::filesystem::path>({"inner", "w.txt"}));
    EXPECT_EQ(names("g/inner"), std::vector<std::filesystem::path>({"y.txt"}));
}

TEST_F(RENAME_PLANNER_TEST, plans_each_folder_alone)
{
    env_.make_folder(env_.root() / "a");
    env_.make_folder(env_.root() / "b");
    const std::vector<RENAME_STEP> steps = plan({request(1, "a/x.txt", U"y.txt"),
        request(2, "b/y.txt", U"x.txt")});
    EXPECT_EQ(kinds(steps), std::vector<STEP_KIND>({STEP_KIND::direct, STEP_KIND::direct}));
    execute(steps);
    EXPECT_EQ(names("a"), std::vector<std::filesystem::path>({"y.txt"}));
    EXPECT_EQ(names("b"), std::vector<std::filesystem::path>({"x.txt"}));
}
