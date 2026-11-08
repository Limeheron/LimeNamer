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

TEST_F(MOD_APPLIER_TEST, formats_upper_and_lower_case)
{
    EXPECT_EQ(apply(R"({"kind":"format","position":{"type":"all"},"format":{"type":"upper"}})",
        U"Photo_é.jpg", U""), U"PHOTO_É.jpg");
    EXPECT_EQ(apply(R"({"kind":"format","part":"ext","position":{"type":"all"},
        "format":{"type":"lower"}})", U"IMG_0001.JPG", U""), U"IMG_0001.jpg");
    EXPECT_EQ(apply(R"({"kind":"format","part":"full","position":{"type":"all"},
        "format":{"type":"upper"}})", U"a.b", U""), U"A.B");
}

TEST_F(MOD_APPLIER_TEST, formats_each_word)
{
    const char *json =
        R"({"kind":"format","position":{"type":"all"},"format":{"type":"title"}})";
    EXPECT_EQ(apply(json, U"hello world_test-case.txt", U""), U"Hello World_Test-Case.txt");
    EXPECT_EQ(apply(json, U"iPhone PHOTO.jpg", U""), U"Iphone Photo.jpg");
    EXPECT_EQ(apply(json, U"don't stop.mp3", U""), U"Don't Stop.mp3");
    EXPECT_EQ(apply(json, U"don’t stop.mp3", U""), U"Don’t Stop.mp3");
    EXPECT_EQ(apply(json, U"rock 'n' roll.mp3", U""), U"Rock 'N' Roll.mp3");
    EXPECT_EQ(apply(json, U"2nd place.txt", U""), U"2nd Place.txt");
    EXPECT_EQ(apply(json, U"élan vital.txt", U""), U"Élan Vital.txt");
    EXPECT_EQ(apply(json, U"한글 file.txt", U""), U"한글 File.txt");
    EXPECT_EQ(apply(json, U"(draft) memo.txt", U""), U"(Draft) Memo.txt");
}

TEST_F(MOD_APPLIER_TEST, formats_the_first_word_only)
{
    const char *json =
        R"({"kind":"format","position":{"type":"all"},"format":{"type":"sentence"}})";
    EXPECT_EQ(apply(json, U"HELLO WORLD. AGAIN.txt", U""), U"Hello world. again.txt");
    EXPECT_EQ(apply(json, U"2024 TRIP.txt", U""), U"2024 trip.txt");
    EXPECT_EQ(apply(json, U"_hello World.txt", U""), U"_Hello world.txt");
}

TEST_F(MOD_APPLIER_TEST, pads_digits)
{
    const char *json = R"({"kind":"format","position":{"type":"all"},
        "format":{"type":"pad_digits","digits":3}})";
    EXPECT_EQ(apply(json, U"IMG_1_23_4567.jpg", U""), U"IMG_001_023_4567.jpg");
    EXPECT_EQ(apply(json, U"photo.jpg", U""), U"photo.jpg");
    EXPECT_EQ(apply(json, U"7.mp3", U""), U"007.mp3");
    EXPECT_EQ(apply(json, U"track１.mp3", U""), U"track１.mp3");
    EXPECT_EQ(apply(R"({"kind":"format","position":{"type":"all"},
        "format":{"type":"pad_digits","digits":1}})", U"a1.txt", U""), U"a1.txt");
}

TEST_F(MOD_APPLIER_TEST, formats_only_the_position)
{
    EXPECT_EQ(apply(R"({"kind":"format","position":{"type":"text","text":"img",
        "occurrence":"all"},"format":{"type":"upper"}})", U"img_x_img.png", U""),
        U"IMG_x_IMG.png");
    EXPECT_EQ(apply(R"({"kind":"format","position":{"type":"first_n","n":3},
        "format":{"type":"pad_digits","digits":2}})", U"1a23.txt", U""), U"01a23.txt");
    EXPECT_EQ(apply(R"({"kind":"format","position":{"type":"first_n","n":3},
        "format":{"type":"pad_digits","digits":3}})", U"1a23.txt", U""), U"001a023.txt");
    EXPECT_EQ(apply(R"({"kind":"format","position":{"type":"last_n","n":2},
        "format":{"type":"pad_digits","digits":5}})", U"123.txt", U""), U"123.txt");
    EXPECT_EQ(apply(R"({"kind":"format","position":{"type":"from_nth","n":2,"count":2},
        "format":{"type":"pad_digits","digits":7}})", U"a12345.txt", U""), U"a0012345.txt");
    EXPECT_EQ(apply(R"({"kind":"format","position":{"type":"between","start_text":"[",
        "end_text":"]"},"format":{"type":"upper"}})", U"a[bc]d.txt", U""), U"a[BC]d.txt");
}

TEST_F(MOD_APPLIER_TEST, skips_extension_format_for_folders)
{
    EXPECT_EQ(apply(R"({"kind":"format","part":"ext","position":{"type":"all"},
        "format":{"type":"upper"}})", U"my.folder", U"", true), U"my.folder");
    EXPECT_EQ(apply(R"({"kind":"format","position":{"type":"all"},"format":{"type":"upper"}})",
        U"my.folder", U"", true), U"MY.FOLDER");
}
