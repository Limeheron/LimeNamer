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

TEST_F(CONFLICT_RESOLVER_TEST, leaves_names_alone_when_disabled)
{
    existing("a", "x.txt");
    existing("a", "y.txt");
    const std::vector<CONFLICT_ENTRY> entries = {
        entry(1, "a", U"x.txt", U"same.txt"), entry(2, "a", U"y.txt", U"same.txt")};
    DUPLICATE_OPTIONS options;
    cache_.load(env_.root() / "a");
    for (const std::optional<std::u32string> &name : resolver_.adjust(entries, cache_, options)) {
        EXPECT_FALSE(name.has_value());
    }
}

TEST_F(CONFLICT_RESOLVER_TEST, numbers_all_but_the_first)
{
    existing("a", "x.jpg");
    existing("a", "y.jpg");
    existing("a", "z.jpg");
    const std::vector<CONFLICT_ENTRY> entries = {entry(1, "a", U"x.jpg", U"사진.jpg"),
        entry(2, "a", U"y.jpg", U"사진.jpg"), entry(3, "a", U"z.jpg", U"사진.jpg")};
    EXPECT_EQ(adjust(entries), std::vector<std::u32string>({U"사진.jpg",
        U"사진 (1).jpg", U"사진 (2).jpg"}));
    EXPECT_EQ(adjust(entries, DUPLICATE_MODE::number_all), std::vector<std::u32string>({
        U"사진 (1).jpg", U"사진 (2).jpg", U"사진 (3).jpg"}));
}

TEST_F(CONFLICT_RESOLVER_TEST, keeps_existing_items_first)
{
    existing("a", "x.jpg");
    existing("a", "y.jpg");
    existing("a", "photo.jpg");
    const std::vector<CONFLICT_ENTRY> entries = {
        entry(1, "a", U"x.jpg", U"photo.jpg"), entry(2, "a", U"y.jpg", U"photo.jpg")};
    const std::vector<std::u32string> expected = {U"photo (1).jpg", U"photo (2).jpg"};
    EXPECT_EQ(adjust(entries), expected);
    EXPECT_EQ(adjust(entries, DUPLICATE_MODE::number_all), expected);
}

TEST_F(CONFLICT_RESOLVER_TEST, keeps_unchanged_items_first)
{
    existing("a", "photo.jpg");
    existing("a", "x.jpg");
    EXPECT_EQ(adjust({entry(1, "a", U"photo.jpg", U"photo.jpg"),
                  entry(2, "a", U"x.jpg", U"PHOTO.jpg")}),
        std::vector<std::u32string>({U"photo.jpg", U"PHOTO (1).jpg"}));
}

TEST_F(CONFLICT_RESOLVER_TEST, uses_each_number_format)
{
    existing("a", "x.jpg");
    existing("a", "y.jpg");
    const std::vector<CONFLICT_ENTRY> entries = {
        entry(1, "a", U"x.jpg", U"p.jpg"), entry(2, "a", U"y.jpg", U"p.jpg")};
    EXPECT_EQ(adjust(entries, DUPLICATE_MODE::keep_first, DUPLICATE_FORMAT::underscore)[1],
        U"p_1.jpg");
    EXPECT_EQ(adjust(entries, DUPLICATE_MODE::keep_first, DUPLICATE_FORMAT::dash)[1], U"p-1.jpg");
    EXPECT_EQ(adjust(entries, DUPLICATE_MODE::keep_first, DUPLICATE_FORMAT::custom, U" v")[1],
        U"p v1.jpg");
}

TEST_F(CONFLICT_RESOLVER_TEST, skips_numbers_already_in_use)
{
    existing("a", "x.jpg");
    existing("a", "y.jpg");
    existing("a", "w.jpg");
    existing("a", "p (1).jpg");
    EXPECT_EQ(adjust({entry(1, "a", U"x.jpg", U"p.jpg"), entry(2, "a", U"y.jpg", U"p.jpg"),
                  entry(3, "a", U"w.jpg", U"p (2).jpg")}),
        std::vector<std::u32string>({U"p.jpg", U"p (3).jpg", U"p (2).jpg"}));
}

TEST_F(CONFLICT_RESOLVER_TEST, numbers_names_without_extensions)
{
    existing("a", "x");
    existing("a", "y");
    existing("a", "z");
    env_.make_folder(env_.root() / "a" / "f1");
    env_.make_folder(env_.root() / "a" / "f2");
    CONFLICT_ENTRY first = entry(3, "a", U"f1", U"my.folder");
    CONFLICT_ENTRY second = entry(4, "a", U"f2", U"my.folder");
    first.is_dir = true;
    second.is_dir = true;
    EXPECT_EQ(adjust({entry(1, "a", U"x", U"README"), entry(2, "a", U"y", U"README"), first,
                  second}),
        std::vector<std::u32string>({U"README", U"README (1)", U"my.folder", U"my.folder (1)"}));
}

TEST_F(CONFLICT_RESOLVER_TEST, adjusts_each_folder_alone)
{
    existing("a", "x.txt");
    existing("b", "y.txt");
    existing("b", "p.txt");
    EXPECT_EQ(adjust({entry(1, "a", U"x.txt", U"p.txt"), entry(2, "b", U"y.txt", U"p.txt")}),
        std::vector<std::u32string>({U"p.txt", U"p (1).txt"}));
}

TEST_F(CONFLICT_RESOLVER_TEST, leaves_freed_names_alone)
{
    existing("a", "x.txt");
    existing("a", "y.txt");
    EXPECT_EQ(adjust({entry(1, "a", U"x.txt", U"y.txt"), entry(2, "a", U"y.txt", U"x.txt")}),
        std::vector<std::u32string>({U"y.txt", U"x.txt"}));
}
