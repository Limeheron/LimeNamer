#include "item_store_test.h"

TEST_F(ITEM_STORE_TEST, adds_files_and_folders)
{
    env_.make_file(at("a.txt"), 3);
    env_.make_folder(at("folder"));
    EXPECT_EQ(store_.add({at("a.txt"), at("folder")}, ADD_MODE::as_items), result(2, 0, 0));
    ASSERT_EQ(store_.size(), size_t{2});
    const ITEM *file = store_.find(store_.order()[0]);
    const ITEM *folder = store_.find(store_.order()[1]);
    EXPECT_EQ(file->name, U"a.txt");
    EXPECT_EQ(file->path, at("a.txt"));
    EXPECT_EQ(file->folder, env_.root());
    EXPECT_EQ(file->info.size, 3);
    EXPECT_FALSE(file->info.is_dir);
    EXPECT_EQ(folder->name, U"folder");
    EXPECT_TRUE(folder->info.is_dir);
    EXPECT_LT(file->id, folder->id);
}

TEST_F(ITEM_STORE_TEST, keeps_the_given_order)
{
    env_.make_file(at("b.txt"), 1);
    env_.make_file(at("a.txt"), 1);
    env_.make_file(at("c.txt"), 1);
    store_.add({at("b.txt"), at("a.txt")}, ADD_MODE::as_items);
    store_.add({at("c.txt")}, ADD_MODE::as_items);
    EXPECT_EQ(names(), std::vector<std::u32string>({U"b.txt", U"a.txt", U"c.txt"}));
}

TEST_F(ITEM_STORE_TEST, skips_items_already_listed)
{
    env_.make_file(at("a.txt"), 1);
    env_.make_folder(at("sub"));
    EXPECT_EQ(store_.add({at("a.txt"), at("a.txt")}, ADD_MODE::as_items), result(1, 1, 0));
    EXPECT_EQ(store_.add({at("A.TXT")}, ADD_MODE::as_items), result(0, 1, 0));
    EXPECT_EQ(store_.add({at("sub/../a.txt")}, ADD_MODE::as_items), result(0, 1, 0));
    EXPECT_EQ(store_.size(), size_t{1});
}

TEST_F(ITEM_STORE_TEST, fails_for_unusable_paths)
{
    EXPECT_EQ(store_.add({at("missing.txt")}, ADD_MODE::as_items), result(0, 0, 1));
    EXPECT_EQ(store_.add({std::filesystem::path("relative.txt")}, ADD_MODE::as_items),
        result(0, 0, 1));
    EXPECT_EQ(store_.add({env_.root().root_path()}, ADD_MODE::as_items), result(0, 0, 1));
    EXPECT_EQ(store_.size(), size_t{0});
}

TEST_F(ITEM_STORE_TEST, accepts_a_trailing_separator)
{
    env_.make_folder(at("folder"));
    std::filesystem::path path = at("folder");
    path += std::filesystem::path::preferred_separator;
    EXPECT_EQ(store_.add({path}, ADD_MODE::as_items), result(1, 0, 0));
    EXPECT_EQ(names(), std::vector<std::u32string>({U"folder"}));
}

TEST_F(ITEM_STORE_TEST, normalizes_names_but_keeps_paths)
{
    const std::filesystem::path nfd = at(u8"é.txt");
    env_.make_file(nfd, 1);
    store_.add({nfd}, ADD_MODE::as_items);
    const ITEM *item = store_.find(store_.order()[0]);
    EXPECT_EQ(item->name, U"é.txt");
    EXPECT_EQ(item->path, nfd);
}

TEST_F(ITEM_STORE_TEST, removes_items)
{
    env_.make_file(at("a.txt"), 1);
    env_.make_file(at("b.txt"), 1);
    env_.make_file(at("c.txt"), 1);
    store_.add({at("a.txt"), at("b.txt"), at("c.txt")}, ADD_MODE::as_items);
    const uint64_t a = store_.order()[0];
    const uint64_t c = store_.order()[2];
    EXPECT_EQ(store_.remove({c, a, 999}), size_t{2});
    EXPECT_EQ(names(), std::vector<std::u32string>({U"b.txt"}));
    EXPECT_FALSE(store_.contains(a));
    EXPECT_TRUE(store_.contains(store_.order()[0]));
    EXPECT_NE(store_.find(a), nullptr);
    EXPECT_EQ(store_.remove({a}), size_t{0});
}

TEST_F(ITEM_STORE_TEST, adds_again_after_removal)
{
    env_.make_file(at("a.txt"), 1);
    store_.add({at("a.txt")}, ADD_MODE::as_items);
    const uint64_t first = store_.order()[0];
    store_.remove({first});
    EXPECT_EQ(store_.add({at("a.txt")}, ADD_MODE::as_items), result(1, 0, 0));
    EXPECT_GT(store_.order()[0], first);
    EXPECT_TRUE(store_.contains(store_.order()[0]));
    EXPECT_FALSE(store_.contains(first));
}

TEST_F(ITEM_STORE_TEST, clears_the_list)
{
    env_.make_file(at("a.txt"), 1);
    store_.add({at("a.txt")}, ADD_MODE::as_items);
    store_.clear();
    EXPECT_EQ(store_.size(), size_t{0});
    EXPECT_EQ(store_.add({at("a.txt")}, ADD_MODE::as_items), result(1, 0, 0));
}

TEST_F(ITEM_STORE_TEST, adds_visible_files_of_a_folder)
{
    env_.make_folder(at("photos"));
    env_.make_file(at("photos/b10.jpg"), 1);
    env_.make_file(at("photos/b2.jpg"), 1);
    env_.make_file(at("photos/B1.jpg"), 1);
    env_.make_file(at("photos/.DS_Store"), 1);
    env_.fake().add_file(at("photos/thumbs.db"), 1, true);
    env_.make_folder(at("photos/sub"));
    env_.make_file(at("photos/sub/inner.jpg"), 1);
    EXPECT_EQ(store_.add({at("photos")}, ADD_MODE::folder_contents), result(3, 0, 0));
    EXPECT_EQ(names(), std::vector<std::u32string>({U"B1.jpg", U"b2.jpg", U"b10.jpg"}));
}

TEST_F(ITEM_STORE_TEST, skips_listed_files_of_a_folder)
{
    env_.make_folder(at("photos"));
    env_.make_file(at("photos/a.jpg"), 1);
    env_.make_file(at("photos/b.jpg"), 1);
    store_.add({at("photos/b.jpg")}, ADD_MODE::as_items);
    EXPECT_EQ(store_.add({at("photos")}, ADD_MODE::folder_contents), result(1, 1, 0));
    EXPECT_EQ(names(), std::vector<std::u32string>({U"b.jpg", U"a.jpg"}));
}

TEST_F(ITEM_STORE_TEST, fails_for_folder_contents_of_non_folders)
{
    env_.make_file(at("a.txt"), 1);
    EXPECT_EQ(store_.add({at("a.txt"), at("missing")}, ADD_MODE::folder_contents),
        result(0, 0, 2));
}

#if defined(_WIN32)
TEST(ITEM_STORE, rejects_names_with_unpaired_surrogates)
{
    REAL_FS_ENV env;
    std::wstring name = L"bad";
    name.push_back(static_cast<wchar_t>(0xD800));
    name += L".txt";
    const std::filesystem::path bad = env.root() / name;
    const HANDLE handle = CreateFileW(bad.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    ASSERT_NE(handle, INVALID_HANDLE_VALUE);
    CloseHandle(handle);
    env.make_file(env.root() / "good.txt", 1);
    std::unique_ptr<TEXT_SERVICE> text_service = PLATFORM::create_text_service();
    ITEM_STORE store(env.file_system(), *text_service);
    ADD_RESULT expected;
    expected.failed = 1;
    EXPECT_EQ(store.add({bad}, ADD_MODE::as_items), expected);
    expected.added = 1;
    EXPECT_EQ(store.add({env.root()}, ADD_MODE::folder_contents), expected);
}
#endif

TEST_F(ITEM_STORE_TEST, moves_an_item_into_a_gap)
{
    const std::vector<uint64_t> ids = add_files({U"a", U"b", U"c", U"d"});
    EXPECT_EQ(store_.move({ids[0]}, ids[2], true, false), true);
    EXPECT_EQ(names(), std::vector<std::u32string>({U"b", U"c", U"a", U"d"}));
    EXPECT_EQ(store_.move({ids[3]}, ids[1], false, false), true);
    EXPECT_EQ(names(), std::vector<std::u32string>({U"d", U"b", U"c", U"a"}));
}

TEST_F(ITEM_STORE_TEST, moves_selected_items_together)
{
    const std::vector<uint64_t> ids = add_files({U"a", U"b", U"c", U"d", U"e"});
    EXPECT_EQ(store_.move({ids[3], ids[0]}, ids[2], false, false), true);
    EXPECT_EQ(names(), std::vector<std::u32string>({U"b", U"a", U"d", U"c", U"e"}));
    EXPECT_EQ(store_.move({ids[1], ids[4]}, ids[0], false, false), true);
    EXPECT_EQ(names(), std::vector<std::u32string>({U"b", U"e", U"a", U"d", U"c"}));
}

TEST_F(ITEM_STORE_TEST, moves_onto_a_selected_item)
{
    const std::vector<uint64_t> ids = add_files({U"a", U"b", U"c", U"d"});
    EXPECT_EQ(store_.move({ids[0], ids[2]}, ids[2], true, false), true);
    EXPECT_EQ(names(), std::vector<std::u32string>({U"b", U"a", U"c", U"d"}));
}

TEST_F(ITEM_STORE_TEST, reports_gaps_that_change_nothing)
{
    const std::vector<uint64_t> ids = add_files({U"a", U"b", U"c"});
    EXPECT_EQ(store_.move({ids[1]}, ids[0], true, true), false);
    EXPECT_EQ(store_.move({ids[1]}, ids[1], false, true), false);
    EXPECT_EQ(store_.move({ids[1]}, ids[1], true, true), false);
    EXPECT_EQ(store_.move({ids[1]}, ids[2], false, true), false);
    EXPECT_EQ(store_.move({ids[0], ids[1]}, ids[2], false, true), false);
    EXPECT_EQ(store_.move({ids[1]}, ids[2], true, true), true);
    EXPECT_EQ(names(), std::vector<std::u32string>({U"a", U"b", U"c"}));
    EXPECT_EQ(store_.sort_state(), state(SORT_KEY::added, false));
}

TEST_F(ITEM_STORE_TEST, rejects_moves_with_unknown_items)
{
    const std::vector<uint64_t> ids = add_files({U"a", U"b"});
    EXPECT_FALSE(store_.move({ids[0]}, 999, true, false).has_value());
    EXPECT_FALSE(store_.move({999}, ids[1], true, false).has_value());
    EXPECT_FALSE(store_.move({}, ids[1], true, false).has_value());
    store_.remove({ids[1]});
    EXPECT_FALSE(store_.move({ids[0]}, ids[1], true, false).has_value());
}

TEST_F(ITEM_STORE_TEST, sorts_by_name)
{
    add_files({U"file10", U"File2", U"file1", U"b"});
    store_.sort(SORT_KEY::name_natural, false);
    EXPECT_EQ(names(), std::vector<std::u32string>({U"b", U"file1", U"File2", U"file10"}));
    EXPECT_EQ(store_.sort_state(), state(SORT_KEY::name_natural, false));
    store_.sort(SORT_KEY::name_natural, true);
    EXPECT_EQ(names(), std::vector<std::u32string>({U"file10", U"File2", U"file1", U"b"}));
    store_.sort(SORT_KEY::name_ordinal, false);
    EXPECT_EQ(names(), std::vector<std::u32string>({U"b", U"file1", U"file10", U"File2"}));
    EXPECT_EQ(store_.sort_state(), state(SORT_KEY::name_ordinal, false));
}

TEST_F(ITEM_STORE_TEST, sorts_by_modified_time)
{
    const std::vector<std::u32string> files = {U"a", U"b", U"c", U"d"};
    const int64_t times[] = {300, 100, 300, 200};
    std::vector<std::filesystem::path> paths;
    for (size_t index = 0; index < files.size(); ++index) {
        const std::filesystem::path path = env_.root() / UTF::path_from_u32(files[index]);
        env_.make_file(path, 1);
        env_.fake().set_modified(path, times[index]);
        paths.push_back(path);
    }
    store_.add(paths, ADD_MODE::as_items);
    store_.sort(SORT_KEY::modified, false);
    EXPECT_EQ(names(), std::vector<std::u32string>({U"b", U"d", U"a", U"c"}));
    store_.sort(SORT_KEY::modified, true);
    EXPECT_EQ(names(), std::vector<std::u32string>({U"c", U"a", U"d", U"b"}));
}

TEST_F(ITEM_STORE_TEST, sorts_back_to_added_order)
{
    add_files({U"c", U"a", U"b"});
    store_.sort(SORT_KEY::name_natural, false);
    store_.sort(SORT_KEY::added, false);
    EXPECT_EQ(names(), std::vector<std::u32string>({U"c", U"a", U"b"}));
    store_.sort(SORT_KEY::added, true);
    EXPECT_EQ(names(), std::vector<std::u32string>({U"b", U"a", U"c"}));
    EXPECT_THROW(store_.sort(SORT_KEY::custom, false), std::invalid_argument);
}

TEST_F(ITEM_STORE_TEST, tracks_the_sort_state)
{
    const std::vector<uint64_t> ids = add_files({U"b", U"a"});
    EXPECT_EQ(store_.sort_state(), state(SORT_KEY::added, false));
    add_files({U"c"});
    EXPECT_EQ(store_.sort_state(), state(SORT_KEY::added, false));
    store_.sort(SORT_KEY::name_natural, false);
    store_.remove({ids[0]});
    EXPECT_EQ(store_.sort_state(), state(SORT_KEY::name_natural, false));
    add_files({U"d"});
    EXPECT_EQ(store_.sort_state(), state(SORT_KEY::custom, false));
    store_.sort(SORT_KEY::name_natural, false);
    store_.move({store_.order()[0]}, store_.order()[1], true, false);
    EXPECT_EQ(store_.sort_state(), state(SORT_KEY::custom, false));
    store_.clear();
    EXPECT_EQ(store_.sort_state(), state(SORT_KEY::added, false));
}

TEST_F(ITEM_STORE_TEST, restores_an_earlier_order)
{
    const std::vector<uint64_t> ids = add_files({U"a", U"b", U"c"});
    const SORT_STATE sorted = state(SORT_KEY::name_natural, true);
    store_.remove({ids[1]});
    store_.restore({ids[2], ids[1], ids[0]}, sorted);
    EXPECT_EQ(names(), std::vector<std::u32string>({U"c", U"b", U"a"}));
    EXPECT_EQ(store_.sort_state(), sorted);
    EXPECT_TRUE(store_.contains(ids[1]));
    EXPECT_EQ(store_.add({at("b")}, ADD_MODE::as_items), result(0, 1, 0));
    store_.restore({ids[0]}, SORT_STATE());
    EXPECT_FALSE(store_.contains(ids[2]));
    EXPECT_EQ(store_.add({at("c")}, ADD_MODE::as_items), result(1, 0, 0));
    EXPECT_THROW(store_.restore({999}, SORT_STATE()), std::invalid_argument);
}

TEST_F(ITEM_STORE_TEST, prunes_items_outside_the_list)
{
    const std::vector<uint64_t> ids = add_files({U"a", U"b"});
    store_.remove({ids[0]});
    store_.prune();
    EXPECT_EQ(store_.find(ids[0]), nullptr);
    EXPECT_NE(store_.find(ids[1]), nullptr);
    EXPECT_THROW(store_.restore({ids[0]}, SORT_STATE()), std::invalid_argument);
}
