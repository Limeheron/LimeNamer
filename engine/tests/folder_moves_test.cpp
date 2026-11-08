#include "folder_moves_test.h"

TEST_F(FOLDER_MOVES_TEST, follows_the_deepest_moved_folder)
{
    moves_.add(at("a"), at("b"));
    moves_.add(at("a/c"), at("b/d"));
    EXPECT_EQ(moves_.follow(at("a/x.txt")), at("b/x.txt"));
    EXPECT_EQ(moves_.follow(at("a/c/y.txt")), at("b/d/y.txt"));
    EXPECT_EQ(moves_.follow(at("a/c/e/z.txt")), at("b/d/e/z.txt"));
}

TEST_F(FOLDER_MOVES_TEST, leaves_other_paths)
{
    EXPECT_TRUE(moves_.empty());
    EXPECT_FALSE(moves_.follow(at("a/x.txt")).has_value());
    moves_.add(at("a"), at("b"));
    EXPECT_FALSE(moves_.empty());
    EXPECT_FALSE(moves_.follow(at("a")).has_value());
    EXPECT_FALSE(moves_.follow(at("ab/x.txt")).has_value());
    EXPECT_FALSE(moves_.follow(at("x.txt")).has_value());
}

TEST_F(FOLDER_MOVES_TEST, applies_swapped_folders_once)
{
    moves_.add(at("a"), at("b"));
    moves_.add(at("b"), at("a"));
    EXPECT_EQ(moves_.follow(at("a/x.txt")), at("b/x.txt"));
    EXPECT_EQ(moves_.follow(at("b/x.txt")), at("a/x.txt"));
}
