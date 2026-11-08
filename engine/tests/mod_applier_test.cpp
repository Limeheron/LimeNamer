#include "mod_applier_test.h"

TEST_F(MOD_APPLIER_TEST, inserts_into_the_name)
{
    EXPECT_EQ(apply(R"({"kind":"insert","position":{"type":"start"},"value":[]})",
        U"photo.jpg", U"2024_"), U"2024_photo.jpg");
    EXPECT_EQ(apply(R"({"kind":"insert","position":{"type":"end"},"value":[]})",
        U"photo.jpg", U"a"), U"photoa.jpg");
    EXPECT_EQ(apply(R"({"kind":"insert","position":{"type":"after_nth","n":3},"value":[]})",
        U"IMG0001.jpg", U"_"), U"IMG_0001.jpg");
    EXPECT_EQ(apply(R"({"kind":"insert","position":{"type":"before_text","text":"_",
        "occurrence":"all"},"value":[]})", U"a_b_c.txt", U"-"), U"a-_b-_c.txt");
}

TEST_F(MOD_APPLIER_TEST, inserts_into_the_extension)
{
    EXPECT_EQ(apply(R"({"kind":"insert","part":"ext","position":{"type":"end"},"value":[]})",
        U"photo.jp", U"g"), U"photo.jpg");
    EXPECT_EQ(apply(R"({"kind":"insert","part":"ext","position":{"type":"end"},"value":[]})",
        U"README", U"txt"), U"README.txt");
}

TEST_F(MOD_APPLIER_TEST, removes_text)
{
    EXPECT_EQ(apply(R"({"kind":"delete","position":{"type":"first_n","n":4}})",
        U"IMG_0001.jpg", U""), U"0001.jpg");
    EXPECT_EQ(apply(R"({"kind":"delete","position":{"type":"text","text":"_",
        "occurrence":"all"}})", U"a_b_c.txt", U""), U"abc.txt");
    EXPECT_EQ(apply(R"({"kind":"delete","position":{"type":"between","start_text":"[",
        "end_text":"]","include_delimiters":true}})", U"photo[2024]_final.png", U""),
        U"photo_final.png");
    EXPECT_EQ(apply(R"({"kind":"delete","position":{"type":"all"}})", U"photo.jpg", U""),
        U".jpg");
}

TEST_F(MOD_APPLIER_TEST, removes_the_extension_with_its_dot)
{
    const char *json = R"({"kind":"delete","part":"ext","position":{"type":"all"}})";
    EXPECT_EQ(apply(json, U"photo.jpg", U""), U"photo");
    EXPECT_EQ(apply(json, U"README", U""), U"README");
}

TEST_F(MOD_APPLIER_TEST, replaces_text)
{
    EXPECT_EQ(apply(R"({"kind":"replace","position":{"type":"text","text":"old"},"value":[]})",
        U"old_file_old.txt", U"new"), U"new_file_old.txt");
    EXPECT_EQ(apply(R"({"kind":"replace","position":{"type":"text","text":"_",
        "occurrence":"all"},"value":[]})", U"a_b_c.txt", U" "), U"a b c.txt");
    EXPECT_EQ(apply(R"({"kind":"replace","position":{"type":"all"},"value":[]})",
        U"IMG_0001.jpg", U"여행_001"), U"여행_001.jpg");
    EXPECT_EQ(apply(R"({"kind":"replace","position":{"type":"between","start_text":"[",
        "end_text":"]"},"value":[]})", U"x[]y", U"1"), U"x[1]y");
}

TEST_F(MOD_APPLIER_TEST, follows_spec_examples)
{
    EXPECT_EQ(apply(R"({"kind":"replace","part":"ext","position":{"type":"all"},"value":[]})",
        U"photo.jpg", U"jpeg"), U"photo.jpeg");
    EXPECT_EQ(apply(R"({"kind":"replace","part":"ext","position":{"type":"all"},"value":[]})",
        U"README", U"jpeg"), U"README.jpeg");
    EXPECT_EQ(apply(R"({"kind":"replace","part":"full","position":{"type":"last_n","n":6},
        "value":[]})", U"backup.tar.gz", U"zip"), U"backup.zip");
}

TEST_F(MOD_APPLIER_TEST, keeps_the_name_without_a_match)
{
    EXPECT_EQ(apply(R"({"kind":"replace","position":{"type":"text","text":"x"},"value":[]})",
        U"photo.jpg", U"y"), U"photo.jpg");
    EXPECT_EQ(apply(R"({"kind":"delete","position":{"type":"from_nth","n":9,"count":1}})",
        U"photo.jpg", U""), U"photo.jpg");
    EXPECT_EQ(apply(R"({"kind":"delete","part":"ext","position":{"type":"first_n","n":2}})",
        U"README", U""), U"README");
    EXPECT_EQ(apply(R"({"kind":"delete","position":{"type":"first_n","n":2}})",
        U".jpg", U""), U"pg");
}

TEST_F(MOD_APPLIER_TEST, treats_folders_as_names_only)
{
    EXPECT_EQ(apply(R"({"kind":"replace","part":"ext","position":{"type":"all"},"value":[]})",
        U"my.folder", U"txt", true), U"my.folder");
    EXPECT_EQ(apply(R"({"kind":"insert","position":{"type":"end"},"value":[]})",
        U"my.folder", U"_2", true), U"my.folder_2");
    EXPECT_EQ(apply(R"({"kind":"insert","position":{"type":"end"},"value":[]})",
        U"my.file", U"_2", false), U"my_2.file");
}

TEST_F(MOD_APPLIER_TEST, keeps_the_result_in_nfc)
{
    EXPECT_EQ(apply(R"({"kind":"insert","position":{"type":"end"},"value":[]})",
        U"cafe.txt", U"́"), U"café.txt");
}
