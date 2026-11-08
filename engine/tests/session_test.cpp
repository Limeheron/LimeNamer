#include "session_test.h"

namespace
{
const char *const INSERT_A =
    R"({"kind":"insert","position":{"type":"start"},"value":[{"text":"a_"}]})";
const char *const INSERT_B =
    R"({"kind":"insert","position":{"type":"end"},"value":[{"text":"_b"}]})";
}

TEST(SESSION, version_is_x_y_z)
{
    EXPECT_TRUE(std::regex_match(SESSION::version(), std::regex(R"(\d+\.\d+\.\d+)")));
}

TEST(SESSION, keeps_given_services)
{
    std::unique_ptr<TEXT_SERVICE> text_service = PLATFORM::create_text_service();
    std::unique_ptr<FILE_SYSTEM> file_system = PLATFORM::create_file_system();
    const TEXT_SERVICE *text_service_address = text_service.get();
    const FILE_SYSTEM *file_system_address = file_system.get();
    SESSION session(std::move(text_service), std::move(file_system), PLATFORM::WINDOWS_HOST);
    EXPECT_EQ(&session.text_service(), text_service_address);
    EXPECT_EQ(&session.file_system(), file_system_address);
}

TEST(SESSION, rejects_missing_services)
{
    EXPECT_THROW(SESSION(nullptr, PLATFORM::create_file_system(), false), std::invalid_argument);
    EXPECT_THROW(SESSION(PLATFORM::create_text_service(), nullptr, false), std::invalid_argument);
}

TEST_F(SESSION_TEST, undoes_and_redoes_item_changes)
{
    session_->add_items(files({U"x.txt", U"y.txt"}), ADD_MODE::as_items);
    const std::vector<uint64_t> ids = session_->items().order();
    session_->remove_items({ids[0]});
    session_->sort_items(SORT_KEY::name_natural, true);
    EXPECT_EQ(session_->items().order(), std::vector<uint64_t>({ids[1]}));
    EXPECT_TRUE(session_->undo());
    EXPECT_EQ(session_->sort_state().key, SORT_KEY::added);
    EXPECT_TRUE(session_->undo());
    EXPECT_EQ(session_->items().order(), ids);
    EXPECT_TRUE(session_->undo());
    EXPECT_TRUE(session_->items().order().empty());
    EXPECT_FALSE(session_->undo());
    EXPECT_TRUE(session_->redo());
    EXPECT_EQ(session_->items().order(), ids);
}

TEST_F(SESSION_TEST, records_only_real_changes)
{
    session_->add_items({root_ / "missing.txt"}, ADD_MODE::as_items);
    EXPECT_FALSE(session_->can_undo());
    session_->add_items(files({U"x.txt", U"y.txt"}), ADD_MODE::as_items);
    const std::vector<uint64_t> ids = session_->items().order();
    EXPECT_TRUE(session_->undo());
    EXPECT_TRUE(session_->redo());
    EXPECT_EQ(session_->move_items({ids[0]}, ids[1], true, true), true);
    EXPECT_EQ(session_->move_items({ids[0]}, ids[0], true, false), false);
    EXPECT_FALSE(session_->remove_mod(999));
    EXPECT_FALSE(session_->set_mod_enabled(999, false));
    session_->set_options(OPTIONS());
    EXPECT_TRUE(session_->undo());
    EXPECT_FALSE(session_->can_undo());
}

TEST_F(SESSION_TEST, undoes_modification_changes)
{
    session_->add_items(files({U"x.txt"}), ADD_MODE::as_items);
    const uint64_t a = session_->add_mod(mod(INSERT_A));
    const uint64_t b = session_->add_mod(mod(INSERT_B));
    session_->set_mod_enabled(a, false);
    session_->move_mod(b, a, false, false);
    session_->remove_mod(a);
    EXPECT_EQ(new_names(), std::vector<std::u32string>({U"x_b.txt"}));
    for (int step = 0; step < 3; ++step) {
        EXPECT_TRUE(session_->undo());
    }
    EXPECT_EQ(new_names(), std::vector<std::u32string>({U"a_x_b.txt"}));
    EXPECT_TRUE(session_->undo());
    EXPECT_EQ(new_names(), std::vector<std::u32string>({U"a_x.txt"}));
}

TEST_F(SESSION_TEST, undoes_direct_edits_and_options)
{
    session_->add_items(files({U"x.txt", U"y.txt"}), ADD_MODE::as_items);
    const std::vector<uint64_t> ids = session_->items().order();
    EXPECT_TRUE(session_->direct_edit(ids[0], U"y.txt").has_value());
    EXPECT_FALSE(session_->direct_edit(999, U"z.txt").has_value());
    EXPECT_EQ(session_->preview().conflict, 1u);
    OPTIONS options;
    options.duplicate.enabled = true;
    session_->set_options(options);
    EXPECT_EQ(session_->preview().conflict, 0u);
    EXPECT_TRUE(session_->undo());
    EXPECT_EQ(session_->preview().conflict, 1u);
    EXPECT_TRUE(session_->undo());
    EXPECT_EQ(new_names(), std::vector<std::u32string>({U"x.txt", U"y.txt"}));
}

TEST_F(SESSION_TEST, sees_existing_names_of_listed_folders)
{
    files({U"taken.txt"});
    session_->add_items(files({U"x.txt"}), ADD_MODE::as_items);
    session_->direct_edit(session_->items().order()[0], U"taken.txt");
    EXPECT_EQ(session_->preview().rows[0].conflict, CONFLICT_KIND::existing_item);
}

TEST_F(SESSION_TEST, refreshes_folder_names)
{
    session_->add_items(files({U"x.txt"}), ADD_MODE::as_items);
    session_->direct_edit(session_->items().order()[0], U"late.txt");
    EXPECT_EQ(session_->preview().conflict, 0u);
    files({U"late.txt"});
    EXPECT_EQ(session_->preview().conflict, 0u);
    session_->refresh_folders();
    EXPECT_EQ(session_->preview().conflict, 1u);
}

TEST_F(SESSION_TEST, keeps_folder_names_after_undo)
{
    files({U"taken.txt"});
    session_->add_items(files({U"x.txt"}), ADD_MODE::as_items);
    session_->direct_edit(session_->items().order()[0], U"taken.txt");
    session_->clear_items();
    EXPECT_TRUE(session_->undo());
    EXPECT_EQ(session_->preview().conflict, 1u);
}

TEST_F(SESSION_TEST, shows_examples)
{
    session_->add_items(files({U"x.txt"}), ADD_MODE::as_items);
    const uint64_t a = session_->add_mod(mod(INSERT_A));
    const EXAMPLE example = session_->example(mod(INSERT_B), a).value();
    EXPECT_EQ(example.before, U"x.txt");
    EXPECT_EQ(example.after, U"x_b.txt");
    EXPECT_FALSE(session_->example(mod(INSERT_B), 999).has_value());
}

TEST_F(SESSION_TEST, previews_ten_thousand_items_quickly)
{
    std::vector<std::filesystem::path> paths;
    for (int folder = 0; folder < 10; ++folder) {
        const std::filesystem::path path = root_ / ("folder" + std::to_string(folder));
        fake_->add_folder(path);
        for (int file = 0; file < 1000; ++file) {
            const std::filesystem::path item = path / ("IMG_" + std::to_string(file) + ".JPG");
            fake_->add_file(item, 2048);
            paths.push_back(item);
        }
    }
    const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    EXPECT_EQ(session_->add_items(paths, ADD_MODE::as_items).added, 10000u);
    const std::chrono::steady_clock::time_point added = std::chrono::steady_clock::now();
    session_->add_mod(mod(R"({"kind":"replace","position":{"type":"text","text":"IMG_"},
        "value":[{"text":"trip_"}]})"));
    session_->add_mod(mod(R"({"kind":"format","position":{"type":"all"},
        "format":{"type":"pad_digits","digits":4}})"));
    session_->add_mod(mod(R"({"kind":"format","part":"ext","position":{"type":"all"},
        "format":{"type":"lower"}})"));
    session_->add_mod(mod(R"({"kind":"insert","position":{"type":"end"},
        "value":[{"text":"_"},{"token":"number"}],"tokens":{"number":{"digits":5}}})"));
    session_->add_mod(mod(R"({"kind":"format","position":{"type":"all"},
        "format":{"type":"title"}})"));
    const std::chrono::steady_clock::time_point built = std::chrono::steady_clock::now();
    const PREVIEW preview = session_->preview();
    const std::chrono::steady_clock::time_point done = std::chrono::steady_clock::now();
    EXPECT_EQ(preview.rows[0].new_name, U"Trip_0000_00001.jpg");
    EXPECT_EQ(preview.runnable, 10000u);

    const auto ms = [](auto from, auto to) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(to - from).count();
    };
    RecordProperty("add_ms", std::to_string(ms(start, added)));
    RecordProperty("preview_ms", std::to_string(ms(built, done)));
    std::cout << "add: " << ms(start, added) << " ms, preview: " << ms(built, done) << " ms\n";
#if defined(NDEBUG)
    EXPECT_LT(ms(built, done), 2000);
#endif
}

TEST_F(SESSION_TEST, adds_mods_as_regular_mods)
{
    session_->add_mod(mod(R"({"kind":"replace","position":{"type":"all"},
        "value":[{"text":"abc"}],"scope":"items","targets":[1],"direct":true})"));
    EXPECT_FALSE(session_->mods()[0].mod.direct);
}
