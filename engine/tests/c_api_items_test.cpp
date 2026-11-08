#include "c_api_items_test.h"

TEST_F(C_API_TEST, adds_items_and_counts_results)
{
    const std::string a = make_file("a.txt");
    const std::string b = make_file("\xea\xb0\x80.txt");
    LN_ADD_RESULT result{};
    ASSERT_EQ(add({a, b, utf8(root_ / "missing.txt")}, ln_add_mode_as_items, result),
        ln_result_ok);
    EXPECT_EQ(result.added, 2u);
    EXPECT_EQ(result.skipped, 0u);
    EXPECT_EQ(result.failed, 1u);
    ASSERT_EQ(add({a}, ln_add_mode_as_items, result), ln_result_ok);
    EXPECT_EQ(result.skipped, 1u);
    EXPECT_EQ(names(), std::vector<std::string>({"a.txt", "\xea\xb0\x80.txt"}));
}

TEST_F(C_API_TEST, adds_folder_contents)
{
    const std::string folder = make_folder("f");
    make_file("f/10.txt");
    make_file("f/2.txt");
    make_file("f/.hidden");
    make_folder("f/sub");
    LN_ADD_RESULT result{};
    ASSERT_EQ(add({folder}, ln_add_mode_folder_contents, result), ln_result_ok);
    EXPECT_EQ(result.added, 2u);
    EXPECT_EQ(names(), std::vector<std::string>({"2.txt", "10.txt"}));
}

TEST_F(C_API_TEST, rejects_bad_add_arguments)
{
    const std::string a = make_file("a.txt");
    const char *const paths[] = {a.c_str()};
    const char *const broken[] = {"\xff"};
    LN_ADD_RESULT result{};
    EXPECT_EQ(ln_items_add(nullptr, paths, 1, ln_add_mode_as_items, &result),
        ln_result_invalid_argument);
    EXPECT_EQ(ln_items_add(session_, paths, 1, ln_add_mode_as_items, nullptr),
        ln_result_invalid_argument);
    EXPECT_EQ(ln_items_add(session_, nullptr, 1, ln_add_mode_as_items, &result),
        ln_result_invalid_argument);
    EXPECT_EQ(ln_items_add(session_, broken, 1, ln_add_mode_as_items, &result),
        ln_result_invalid_argument);
    EXPECT_EQ(ln_items_add(session_, paths, 1, 2, &result), ln_result_invalid_argument);
    EXPECT_TRUE(names().empty());
}

TEST_F(C_API_TEST, removes_and_clears_items)
{
    LN_ADD_RESULT result{};
    add({make_file("a.txt"), make_file("b.txt"), make_file("c.txt")}, ln_add_mode_as_items,
        result);
    const std::vector<uint64_t> items = ids();
    const uint64_t unknown[] = {items[1], 999};
    EXPECT_EQ(ln_items_remove(session_, unknown, 2), ln_result_not_found);
    EXPECT_EQ(names().size(), size_t{3});
    EXPECT_EQ(ln_items_remove(session_, nullptr, 1), ln_result_invalid_argument);
    EXPECT_EQ(ln_items_remove(session_, nullptr, 0), ln_result_ok);
    ASSERT_EQ(ln_items_remove(session_, &items[1], 1), ln_result_ok);
    EXPECT_EQ(names(), std::vector<std::string>({"a.txt", "c.txt"}));
    ASSERT_EQ(ln_items_clear(session_), ln_result_ok);
    EXPECT_TRUE(names().empty());
    EXPECT_EQ(ln_items_clear(nullptr), ln_result_invalid_argument);
}

TEST_F(C_API_TEST, moves_items)
{
    LN_ADD_RESULT result{};
    add({make_file("a.txt"), make_file("b.txt"), make_file("c.txt")}, ln_add_mode_as_items,
        result);
    const std::vector<uint64_t> items = ids();
    uint8_t changed = 9;
    ASSERT_EQ(ln_items_move(session_, &items[2], 1, items[0], 0, 1, &changed), ln_result_ok);
    EXPECT_EQ(changed, 1);
    EXPECT_EQ(names(), std::vector<std::string>({"a.txt", "b.txt", "c.txt"}));
    ASSERT_EQ(ln_items_move(session_, &items[2], 1, items[0], 0, 0, &changed), ln_result_ok);
    EXPECT_EQ(names(), std::vector<std::string>({"c.txt", "a.txt", "b.txt"}));
    ASSERT_EQ(ln_items_move(session_, &items[2], 1, items[1], 1, 1, &changed), ln_result_ok);
    EXPECT_EQ(changed, 1);
    ASSERT_EQ(ln_items_move(session_, &items[0], 1, items[2], 1, 1, &changed), ln_result_ok);
    EXPECT_EQ(changed, 0);

    int32_t key = -1;
    uint8_t descending = 9;
    ASSERT_EQ(ln_items_sort_state(session_, &key, &descending), ln_result_ok);
    EXPECT_EQ(key, ln_sort_key_custom);
    EXPECT_EQ(descending, 0);

    EXPECT_EQ(ln_items_move(session_, &items[0], 1, 999, 0, 0, &changed), ln_result_not_found);
    EXPECT_EQ(ln_items_move(session_, nullptr, 0, items[0], 0, 0, &changed),
        ln_result_invalid_argument);
    EXPECT_EQ(ln_items_move(session_, &items[0], 1, items[1], 2, 0, &changed),
        ln_result_invalid_argument);
    EXPECT_EQ(ln_items_move(session_, &items[0], 1, items[1], 0, 0, nullptr),
        ln_result_invalid_argument);
}

TEST_F(C_API_TEST, sorts_items)
{
    LN_ADD_RESULT result{};
    add({make_file("b10.txt"), make_file("B9.txt"), make_file("a.txt")}, ln_add_mode_as_items,
        result);
    ASSERT_EQ(ln_items_sort(session_, ln_sort_key_name_natural, 0), ln_result_ok);
    EXPECT_EQ(names(), std::vector<std::string>({"a.txt", "B9.txt", "b10.txt"}));
    ASSERT_EQ(ln_items_sort(session_, ln_sort_key_name_ordinal, 1), ln_result_ok);
    EXPECT_EQ(names(), std::vector<std::string>({"B9.txt", "b10.txt", "a.txt"}));
    int32_t key = -1;
    uint8_t descending = 9;
    ASSERT_EQ(ln_items_sort_state(session_, &key, &descending), ln_result_ok);
    EXPECT_EQ(key, ln_sort_key_name_ordinal);
    EXPECT_EQ(descending, 1);
    ASSERT_EQ(ln_items_sort(session_, ln_sort_key_added, 0), ln_result_ok);
    EXPECT_EQ(names(), std::vector<std::string>({"b10.txt", "B9.txt", "a.txt"}));

    EXPECT_EQ(ln_items_sort(session_, ln_sort_key_custom, 0), ln_result_invalid_argument);
    EXPECT_EQ(ln_items_sort(session_, -1, 0), ln_result_invalid_argument);
    EXPECT_EQ(ln_items_sort(session_, ln_sort_key_added, 2), ln_result_invalid_argument);
    EXPECT_EQ(ln_items_sort_state(session_, nullptr, &descending), ln_result_invalid_argument);
}

TEST_F(C_API_TEST, refreshes_folders)
{
    EXPECT_EQ(ln_folders_refresh(session_), ln_result_ok);
    EXPECT_EQ(ln_folders_refresh(nullptr), ln_result_invalid_argument);
}

TEST_F(C_API_TEST, previews_items)
{
    LN_ADD_RESULT result{};
    add({make_folder("f"), make_file("a.txt")}, ln_add_mode_as_items, result);
    LN_PREVIEW *preview = nullptr;
    ASSERT_EQ(ln_preview(session_, &preview), ln_result_ok);
    ASSERT_NE(preview, nullptr);
    ASSERT_EQ(preview->count, size_t{2});
    EXPECT_EQ(std::string(preview->rows[0].folder), utf8(root_));
    EXPECT_EQ(preview->rows[0].is_dir, 1);
    EXPECT_EQ(preview->rows[1].size, 1);
    EXPECT_EQ(preview->rows[1].status, ln_status_unchanged);
    EXPECT_EQ(preview->rows[0].folder, preview->rows[1].folder);
    EXPECT_EQ(preview->changed, 0u);
    ln_preview_free(preview);

    preview = reinterpret_cast<LN_PREVIEW *>(&result);
    EXPECT_EQ(ln_preview(nullptr, &preview), ln_result_invalid_argument);
    EXPECT_EQ(preview, nullptr);
    EXPECT_EQ(ln_preview(session_, nullptr), ln_result_invalid_argument);
}
