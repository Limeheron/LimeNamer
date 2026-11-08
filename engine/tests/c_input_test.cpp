#include "c_input_test.h"

TEST(C_INPUT, reads_text)
{
    EXPECT_EQ(C_INPUT::text("\xea\xb0\x80" "a"), std::u32string(U"가a"));
    EXPECT_EQ(C_INPUT::text(""), std::u32string());
    EXPECT_FALSE(C_INPUT::text(nullptr).has_value());
    EXPECT_FALSE(C_INPUT::text("\xff").has_value());
    EXPECT_FALSE(C_INPUT::text("\xed\xa0\x80").has_value());
}

TEST(C_INPUT, reads_paths)
{
    EXPECT_EQ(C_INPUT::path("/a/\xea\xb0\x80"), UTF::path_from_u32(U"/a/가"));
    EXPECT_FALSE(C_INPUT::path(nullptr).has_value());
    EXPECT_FALSE(C_INPUT::path("/a/\xc0\xaf").has_value());

    const char *const values[] = {"/a", "/b"};
    const std::optional<std::vector<std::filesystem::path>> paths = C_INPUT::paths(values, 2);
    ASSERT_TRUE(paths.has_value());
    EXPECT_EQ(*paths, std::vector<std::filesystem::path>({"/a", "/b"}));
    EXPECT_TRUE(C_INPUT::paths(nullptr, 0)->empty());
    EXPECT_FALSE(C_INPUT::paths(nullptr, 1).has_value());
    const char *const broken[] = {"/a", nullptr};
    EXPECT_FALSE(C_INPUT::paths(broken, 2).has_value());
}

TEST(C_INPUT, reads_ids)
{
    const uint64_t values[] = {3, 1};
    EXPECT_EQ(C_INPUT::ids(values, 2), std::vector<uint64_t>({3, 1}));
    EXPECT_TRUE(C_INPUT::ids(nullptr, 0)->empty());
    EXPECT_FALSE(C_INPUT::ids(nullptr, 1).has_value());
}

TEST(C_INPUT, accepts_only_zero_or_one_as_flags)
{
    EXPECT_EQ(C_INPUT::flag(0), false);
    EXPECT_EQ(C_INPUT::flag(1), true);
    EXPECT_FALSE(C_INPUT::flag(2).has_value());
}

TEST(C_INPUT, accepts_only_known_choices)
{
    EXPECT_EQ(C_INPUT::choice(ln_sort_key_modified, SORT_KEY::custom), SORT_KEY::modified);
    EXPECT_EQ(C_INPUT::choice(ln_add_mode_folder_contents, ADD_MODE::folder_contents),
        ADD_MODE::folder_contents);
    EXPECT_EQ(C_INPUT::choice(ln_resolve_choice_revert_all, RESOLVE_CHOICE::keep_succeeded),
        RESOLVE_CHOICE::revert_all);
    EXPECT_FALSE(C_INPUT::choice(-1, SORT_KEY::custom).has_value());
    EXPECT_FALSE(C_INPUT::choice(2, ADD_MODE::folder_contents).has_value());
}
