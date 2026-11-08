#include "folder_cache_test.h"

TEST_F(FOLDER_CACHE_TEST, reads_names_of_a_folder)
{
    env_.make_folder(at("photos"));
    env_.make_file(at("photos/IMG_1.jpg"), 1);
    env_.make_folder(at("photos/sub"));
    cache_.load(at("photos"));
    EXPECT_TRUE(cache_.has_name(at("photos"), U"IMG_1.jpg"));
    EXPECT_TRUE(cache_.has_name(at("photos"), U"img_1.JPG"));
    EXPECT_TRUE(cache_.has_name(at("photos"), U"sub"));
    EXPECT_FALSE(cache_.has_name(at("photos"), U"IMG_2.jpg"));
    EXPECT_FALSE(cache_.has_name(at("other"), U"IMG_1.jpg"));
    ASSERT_NE(cache_.find(at("photos")), nullptr);
    EXPECT_TRUE(cache_.find(at("photos"))->readable);
}

TEST_F(FOLDER_CACHE_TEST, matches_names_in_any_normalization)
{
    env_.make_folder(at("photos"));
    env_.make_file(at(u8"photos/é.txt"), 1);
    cache_.load(at("photos"));
    EXPECT_TRUE(cache_.has_name(at("photos"), U"é.txt"));
    EXPECT_TRUE(cache_.has_name(at("photos"), U"É.TXT"));
}

TEST_F(FOLDER_CACHE_TEST, reads_again_only_when_asked)
{
    env_.make_folder(at("photos"));
    cache_.load(at("photos"));
    env_.make_file(at("photos/new.jpg"), 1);
    cache_.load(at("photos"));
    EXPECT_FALSE(cache_.has_name(at("photos"), U"new.jpg"));
    cache_.reload(at("photos"));
    EXPECT_TRUE(cache_.has_name(at("photos"), U"new.jpg"));
    env_.make_file(at("photos/newer.jpg"), 1);
    cache_.refresh();
    EXPECT_TRUE(cache_.has_name(at("photos"), U"newer.jpg"));
}

TEST_F(FOLDER_CACHE_TEST, marks_unreadable_folders)
{
    cache_.load(at("missing"));
    ASSERT_NE(cache_.find(at("missing")), nullptr);
    EXPECT_FALSE(cache_.find(at("missing"))->readable);
    EXPECT_FALSE(cache_.has_name(at("missing"), U"a.txt"));
}

TEST_F(FOLDER_CACHE_TEST, keeps_only_folders_in_use)
{
    env_.make_folder(at("a"));
    env_.make_folder(at("b"));
    cache_.load(at("a"));
    cache_.load(at("b"));
    EXPECT_EQ(cache_.size(), size_t{2});
    cache_.retain({at("B")});
    EXPECT_EQ(cache_.size(), size_t{1});
    EXPECT_EQ(cache_.find(at("a")), nullptr);
    EXPECT_NE(cache_.find(at("b")), nullptr);
}
