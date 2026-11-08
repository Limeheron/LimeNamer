#include "file_system_test.h"

using FILE_SYSTEM_ENVS = testing::Types<FAKE_FS_ENV, REAL_FS_ENV>;
TYPED_TEST_SUITE(FILE_SYSTEM_TEST, FILE_SYSTEM_ENVS);

TYPED_TEST(FILE_SYSTEM_TEST, get_info_reads_file)
{
    this->env_.make_file(this->at("a.txt"), 5);
    FILE_INFO info;
    ASSERT_TRUE(this->fs().get_info(this->at("a.txt"), info).ok());
    EXPECT_FALSE(info.is_dir);
    EXPECT_FALSE(info.hidden);
    EXPECT_EQ(info.size, 5);
    EXPECT_TRUE(info.id.valid);
}

TYPED_TEST(FILE_SYSTEM_TEST, get_info_reads_folder)
{
    this->env_.make_folder(this->at("folder"));
    FILE_INFO info;
    ASSERT_TRUE(this->fs().get_info(this->at("folder"), info).ok());
    EXPECT_TRUE(info.is_dir);
    EXPECT_EQ(info.size, 0);
    EXPECT_TRUE(info.id.valid);
}

TYPED_TEST(FILE_SYSTEM_TEST, get_info_reports_missing_entries)
{
    FILE_INFO info;
    EXPECT_EQ(this->fs().get_info(this->at("missing.txt"), info).error, FS_ERROR::not_found);
    EXPECT_EQ(this->fs().get_info(this->at("missing/a.txt"), info).error, FS_ERROR::not_found);
}

TYPED_TEST(FILE_SYSTEM_TEST, get_info_ignores_case)
{
    this->env_.make_file(this->at("Photo.JPG"), 3);
    EXPECT_EQ(this->size_of(this->at("photo.jpg")), 3);
}

TYPED_TEST(FILE_SYSTEM_TEST, ids_differ_between_entries)
{
    this->env_.make_file(this->at("a.txt"), 1);
    this->env_.make_file(this->at("b.txt"), 1);
    FILE_INFO a;
    FILE_INFO b;
    ASSERT_TRUE(this->fs().get_info(this->at("a.txt"), a).ok());
    ASSERT_TRUE(this->fs().get_info(this->at("b.txt"), b).ok());
    EXPECT_FALSE(a.id == b.id);
}

TYPED_TEST(FILE_SYSTEM_TEST, list_names_returns_direct_children)
{
    this->env_.make_file(this->at("a.txt"), 1);
    this->env_.make_file(this->at("B.txt"), 1);
    this->env_.make_folder(this->at("c"));
    this->env_.make_file(this->at("c/inner.txt"), 1);
    const std::vector<std::filesystem::path> expected = {"B.txt", "a.txt", "c"};
    EXPECT_EQ(this->sorted_names(this->env_.root()), expected);
}

TYPED_TEST(FILE_SYSTEM_TEST, list_names_of_empty_folder)
{
    this->env_.make_folder(this->at("empty"));
    EXPECT_TRUE(this->sorted_names(this->at("empty")).empty());
}

TYPED_TEST(FILE_SYSTEM_TEST, list_names_reports_missing_folder)
{
    this->env_.make_file(this->at("a.txt"), 1);
    std::vector<std::filesystem::path> names;
    EXPECT_EQ(this->fs().list_names(this->at("missing"), names).error, FS_ERROR::not_found);
    EXPECT_EQ(this->fs().list_names(this->at("a.txt"), names).error, FS_ERROR::not_found);
}

TYPED_TEST(FILE_SYSTEM_TEST, rename_moves_file)
{
    this->env_.make_file(this->at("a.txt"), 7);
    FILE_INFO before;
    ASSERT_TRUE(this->fs().get_info(this->at("a.txt"), before).ok());
    ASSERT_TRUE(this->fs().rename(this->at("a.txt"), this->at("b.txt")).ok());
    FILE_INFO after;
    ASSERT_TRUE(this->fs().get_info(this->at("b.txt"), after).ok());
    EXPECT_FALSE(this->exists(this->at("a.txt")));
    EXPECT_EQ(after.size, 7);
    EXPECT_TRUE(before.id == after.id);
}

TYPED_TEST(FILE_SYSTEM_TEST, rename_keeps_given_case)
{
    this->env_.make_file(this->at("a.txt"), 1);
    ASSERT_TRUE(this->fs().rename(this->at("a.txt"), this->at("MixedCase.TXT")).ok());
    const std::vector<std::filesystem::path> expected = {"MixedCase.TXT"};
    EXPECT_EQ(this->sorted_names(this->env_.root()), expected);
}

TYPED_TEST(FILE_SYSTEM_TEST, rename_rejects_existing_target)
{
    this->env_.make_file(this->at("a.txt"), 1);
    this->env_.make_file(this->at("b.txt"), 2);
    EXPECT_EQ(this->fs().rename(this->at("a.txt"), this->at("b.txt")).error,
        FS_ERROR::already_exists);
    EXPECT_EQ(this->size_of(this->at("a.txt")), 1);
    EXPECT_EQ(this->size_of(this->at("b.txt")), 2);
}

TYPED_TEST(FILE_SYSTEM_TEST, rename_rejects_target_differing_in_case)
{
    this->env_.make_file(this->at("a.txt"), 1);
    this->env_.make_file(this->at("B.txt"), 2);
    EXPECT_EQ(this->fs().rename(this->at("a.txt"), this->at("b.TXT")).error,
        FS_ERROR::already_exists);
    EXPECT_EQ(this->size_of(this->at("a.txt")), 1);
    EXPECT_EQ(this->size_of(this->at("B.txt")), 2);
}

TYPED_TEST(FILE_SYSTEM_TEST, rename_rejects_existing_folder_target)
{
    this->env_.make_folder(this->at("f"));
    this->env_.make_folder(this->at("g"));
    EXPECT_EQ(this->fs().rename(this->at("f"), this->at("g")).error, FS_ERROR::already_exists);
    EXPECT_TRUE(this->exists(this->at("f")));
}

TYPED_TEST(FILE_SYSTEM_TEST, rename_reports_missing_source)
{
    EXPECT_EQ(this->fs().rename(this->at("missing.txt"), this->at("b.txt")).error,
        FS_ERROR::not_found);
    EXPECT_FALSE(this->exists(this->at("b.txt")));
}

TYPED_TEST(FILE_SYSTEM_TEST, rename_reports_missing_target_folder)
{
    this->env_.make_file(this->at("a.txt"), 1);
    EXPECT_EQ(this->fs().rename(this->at("a.txt"), this->at("missing/b.txt")).error,
        FS_ERROR::not_found);
    EXPECT_TRUE(this->exists(this->at("a.txt")));
}

TYPED_TEST(FILE_SYSTEM_TEST, rename_folder_keeps_children)
{
    this->env_.make_folder(this->at("f"));
    this->env_.make_folder(this->at("f/sub"));
    this->env_.make_file(this->at("f/sub/x.txt"), 4);
    FILE_INFO before;
    ASSERT_TRUE(this->fs().get_info(this->at("f/sub/x.txt"), before).ok());
    ASSERT_TRUE(this->fs().rename(this->at("f"), this->at("g")).ok());
    FILE_INFO after;
    ASSERT_TRUE(this->fs().get_info(this->at("g/sub/x.txt"), after).ok());
    EXPECT_EQ(after.size, 4);
    EXPECT_TRUE(before.id == after.id);
    EXPECT_FALSE(this->exists(this->at("f")));
    const std::vector<std::filesystem::path> expected = {"x.txt"};
    EXPECT_EQ(this->sorted_names(this->at("g/sub")), expected);
}

TYPED_TEST(FILE_SYSTEM_TEST, handles_unicode_names)
{
    const std::filesystem::path name = u8"한글 😀.txt";
    const std::filesystem::path renamed = u8"새 이름 é.txt";
    this->env_.make_file(this->env_.root() / name, 2);
    const std::vector<std::filesystem::path> before = {name};
    EXPECT_EQ(this->sorted_names(this->env_.root()), before);
    ASSERT_TRUE(this->fs().rename(this->env_.root() / name, this->env_.root() / renamed).ok());
    const std::vector<std::filesystem::path> after = {renamed};
    EXPECT_EQ(this->sorted_names(this->env_.root()), after);
    EXPECT_EQ(this->size_of(this->env_.root() / renamed), 2);
}
