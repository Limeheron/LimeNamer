#include "name_parts_test.h"

namespace
{
struct SPLIT_CASE
{
    std::u32string_view name;
    std::u32string_view stem;
    std::u32string_view ext;
    bool has_ext;
};

constexpr const SPLIT_CASE FILE_CASES[] = {
    {U"photo.jpg", U"photo", U"jpg", true},
    {U"archive.tar.gz", U"archive.tar", U"gz", true},
    {U"사진.JPG", U"사진", U"JPG", true},
    {U"README", U"README", U"", false},
    {U".gitignore", U".gitignore", U"", false},
    {U".bashrc.bak", U".bashrc", U"bak", true},
    {U"..hidden", U"..hidden", U"", false},
    {U"name.", U"name.", U"", false},
    {U"a..b", U"a.", U"b", true},
    {U"", U"", U"", false},
    {U".", U".", U"", false},
    {U"..", U"..", U"", false},
    {U"photo 1.final.png", U"photo 1.final", U"png", true},
};
}

TEST(NAME_PARTS, splits_file_names)
{
    for (const SPLIT_CASE &item : FILE_CASES) {
        const NAME_PARTS parts = NAME_PARTS::split(item.name, false);
        const std::string label = UTF::encode_utf8(item.name);
        EXPECT_EQ(parts.stem(), item.stem) << label;
        EXPECT_EQ(parts.ext(), item.ext) << label;
        EXPECT_EQ(parts.has_ext(), item.has_ext) << label;
    }
}

TEST(NAME_PARTS, joins_unchanged_parts_back_to_the_name)
{
    for (const SPLIT_CASE &item : FILE_CASES) {
        const NAME_PARTS parts = NAME_PARTS::split(item.name, false);
        const std::string label = UTF::encode_utf8(item.name);
        EXPECT_EQ(parts.join(MOD_PART::name, parts.text_of(MOD_PART::name)), item.name) << label;
        EXPECT_EQ(parts.join(MOD_PART::ext, parts.text_of(MOD_PART::ext)), item.name) << label;
        EXPECT_EQ(parts.join(MOD_PART::full, parts.text_of(MOD_PART::full)), item.name) << label;
    }
}

TEST(NAME_PARTS, gives_text_of_each_part)
{
    const NAME_PARTS parts = NAME_PARTS::split(U"archive.tar.gz", false);
    EXPECT_EQ(parts.text_of(MOD_PART::name), U"archive.tar");
    EXPECT_EQ(parts.text_of(MOD_PART::ext), U"gz");
    EXPECT_EQ(parts.text_of(MOD_PART::full), U"archive.tar.gz");
}

TEST(NAME_PARTS, joins_changed_name)
{
    EXPECT_EQ(NAME_PARTS::split(U"photo.jpg", false).join(MOD_PART::name, U"img"), U"img.jpg");
    EXPECT_EQ(NAME_PARTS::split(U"README", false).join(MOD_PART::name, U"NOTE"), U"NOTE");
    EXPECT_EQ(NAME_PARTS::split(U"photo.jpg", false).join(MOD_PART::name, U""), U".jpg");
}

TEST(NAME_PARTS, adds_and_removes_the_dot_with_the_extension)
{
    EXPECT_EQ(NAME_PARTS::split(U"photo.jpg", false).join(MOD_PART::ext, U"jpeg"),
        U"photo.jpeg");
    EXPECT_EQ(NAME_PARTS::split(U"photo.jpg", false).join(MOD_PART::ext, U""), U"photo");
    EXPECT_EQ(NAME_PARTS::split(U"README", false).join(MOD_PART::ext, U"txt"), U"README.txt");
    EXPECT_EQ(NAME_PARTS::split(U".gitignore", false).join(MOD_PART::ext, U"bak"),
        U".gitignore.bak");
}

TEST(NAME_PARTS, joins_full_name)
{
    EXPECT_EQ(NAME_PARTS::split(U"backup.tar.gz", false).join(MOD_PART::full, U"backup.zip"),
        U"backup.zip");
}

TEST(NAME_PARTS, treats_folder_name_as_a_whole)
{
    const NAME_PARTS parts = NAME_PARTS::split(U"my.folder", true);
    EXPECT_EQ(parts.stem(), U"my.folder");
    EXPECT_TRUE(parts.ext().empty());
    EXPECT_FALSE(parts.has_ext());
    EXPECT_TRUE(parts.applies(MOD_PART::name));
    EXPECT_FALSE(parts.applies(MOD_PART::ext));
    EXPECT_TRUE(parts.applies(MOD_PART::full));
    EXPECT_EQ(parts.join(MOD_PART::name, U"new.folder"), U"new.folder");
    EXPECT_EQ(parts.join(MOD_PART::ext, U"txt"), U"my.folder");
    EXPECT_EQ(parts.join(MOD_PART::full, U"other"), U"other");
}

TEST(NAME_PARTS, applies_every_part_to_files)
{
    const NAME_PARTS parts = NAME_PARTS::split(U"README", false);
    EXPECT_TRUE(parts.applies(MOD_PART::name));
    EXPECT_TRUE(parts.applies(MOD_PART::ext));
    EXPECT_TRUE(parts.applies(MOD_PART::full));
}
