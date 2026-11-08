#include "conflict_resolver_test.h"

TEST_F(CONFLICT_RESOLVER_TEST, accepts_distinct_names)
{
    existing("a", "x.txt");
    existing("a", "y.txt");
    EXPECT_EQ(find({entry(1, "a", U"x.txt", U"x1.txt"), entry(2, "a", U"y.txt", U"y1.txt")}),
        std::vector<CONFLICT>({none(), none()}));
}

TEST_F(CONFLICT_RESOLVER_TEST, finds_items_renamed_to_the_same_name)
{
    existing("a", "x.txt");
    existing("a", "y.txt");
    existing("a", "z.txt");
    EXPECT_EQ(find({entry(1, "a", U"x.txt", U"same.txt"), entry(2, "a", U"y.txt", U"same.txt"),
                  entry(3, "a", U"z.txt", U"SAME.TXT")}),
        std::vector<CONFLICT>({with_item(2), with_item(1), with_item(1)}));
}

TEST_F(CONFLICT_RESOLVER_TEST, finds_names_taken_by_unchanged_items)
{
    existing("a", "keep.txt");
    existing("a", "x.txt");
    EXPECT_EQ(find({entry(1, "a", U"keep.txt", U"keep.txt"),
                  entry(2, "a", U"x.txt", U"Keep.txt")}),
        std::vector<CONFLICT>({none(), with_item(1)}));
}

TEST_F(CONFLICT_RESOLVER_TEST, ignores_items_in_other_folders)
{
    existing("a", "x.txt");
    existing("b", "y.txt");
    EXPECT_EQ(find({entry(1, "a", U"x.txt", U"same.txt"), entry(2, "b", U"y.txt", U"same.txt")}),
        std::vector<CONFLICT>({none(), none()}));
}

TEST_F(CONFLICT_RESOLVER_TEST, finds_existing_items_outside_the_list)
{
    existing("a", "x.txt");
    existing("a", "Taken.txt");
    EXPECT_EQ(find({entry(1, "a", U"x.txt", U"taken.TXT")}),
        std::vector<CONFLICT>({with_existing()}));
}

TEST_F(CONFLICT_RESOLVER_TEST, matches_existing_names_in_any_normalization)
{
    existing("a", "x.txt");
    existing("a", u8"é.txt");
    EXPECT_EQ(find({entry(1, "a", U"x.txt", U"é.txt")}),
        std::vector<CONFLICT>({with_existing()}));
}

TEST_F(CONFLICT_RESOLVER_TEST, allows_names_freed_by_listed_items)
{
    existing("a", "x.txt");
    existing("a", "y.txt");
    EXPECT_EQ(find({entry(1, "a", U"x.txt", U"y.txt"), entry(2, "a", U"y.txt", U"z.txt")}),
        std::vector<CONFLICT>({none(), none()}));
}

TEST_F(CONFLICT_RESOLVER_TEST, allows_swaps_and_cycles)
{
    existing("a", "x.txt");
    existing("a", "y.txt");
    existing("a", "z.txt");
    EXPECT_EQ(find({entry(1, "a", U"x.txt", U"y.txt"), entry(2, "a", U"y.txt", U"x.txt")}),
        std::vector<CONFLICT>({none(), none()}));
    EXPECT_EQ(find({entry(1, "a", U"x.txt", U"y.txt"), entry(2, "a", U"y.txt", U"z.txt"),
                  entry(3, "a", U"z.txt", U"x.txt")}),
        std::vector<CONFLICT>({none(), none(), none()}));
}

TEST_F(CONFLICT_RESOLVER_TEST, allows_case_only_changes)
{
    existing("a", "photo.jpg");
    EXPECT_EQ(find({entry(1, "a", U"photo.jpg", U"Photo.JPG")}),
        std::vector<CONFLICT>({none()}));
}

TEST_F(CONFLICT_RESOLVER_TEST, ignores_unchanged_items)
{
    existing("a", "x.txt");
    existing("a", "taken.txt");
    EXPECT_EQ(find({entry(1, "a", U"x.txt", U"x.txt")}), std::vector<CONFLICT>({none()}));
}

TEST_F(CONFLICT_RESOLVER_TEST, skips_existing_check_for_unreadable_folders)
{
    CONFLICT_ENTRY missing = entry(1, "a", U"x.txt", U"y.txt");
    missing.folder = env_.root() / "missing";
    cache_.load(missing.folder);
    EXPECT_EQ(resolver_.find({missing}, cache_), std::vector<CONFLICT>({none()}));
}
