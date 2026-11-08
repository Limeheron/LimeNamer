#include "mod_cases_test.h"

TEST_F(MOD_CASES_TEST, data_file_has_cases)
{
    const nlohmann::json cases = load_cases();
    ASSERT_TRUE(cases.is_array());
    EXPECT_GE(cases.size(), size_t{10});
}

TEST_F(MOD_CASES_TEST, names_match_expected)
{
    const nlohmann::json cases = load_cases();
    size_t checked = 0;
    for (const nlohmann::json &test_case : cases) {
        const std::string case_name = test_case.at("name").get<std::string>();
        SCOPED_TRACE(case_name);
        FAKE_FS_ENV env;
        ITEM_STORE items(env.file_system(), env.text_service());
        MOD_STORE mods(env.text_service());
        FOLDER_CACHE cache(env.file_system(), env.text_service());
        const NAME_CHECKER checker(WINDOWS_HOST);
        const PREVIEW_BUILDER builder(env.text_service(), checker);

        std::unordered_set<std::string> folders;
        std::vector<std::filesystem::path> paths;
        for (const nlohmann::json &item : test_case.at("items")) {
            const std::string parent = item.value("parent", std::string("items"));
            const std::filesystem::path folder = env.root() / path_of(parent);
            if (folders.insert(parent).second) {
                env.make_folder(folder);
            }
            const std::filesystem::path path = folder / path_of(item.at("name").get<std::string>());
            if (item.value("is_dir", false)) {
                env.make_folder(path);
            } else {
                env.make_file(path, item.value("size", int64_t{0}));
            }
            paths.push_back(path);
            cache.load(folder);
        }
        ASSERT_EQ(items.add(paths, ADD_MODE::as_items).added, paths.size());

        for (const nlohmann::json &mod : test_case.at("mods")) {
            const MODIFICATION parsed = MOD_JSON::parse(mod.dump());
            EXPECT_TRUE(MOD_CHECKER::check(parsed).empty()) << mod.dump();
            mods.add(parsed);
        }
        const PREVIEW preview = builder.build(items, mods.entries(), OPTIONS(), cache);
        const nlohmann::json &expected = test_case.at("items");
        ASSERT_EQ(preview.rows.size(), expected.size());
        for (size_t index = 0; index < expected.size(); ++index) {
            EXPECT_EQ(UTF::encode_utf8(preview.rows[index].new_name),
                expected[index].at("expected").get<std::string>())
                << expected[index].at("name").get<std::string>();
            ++checked;
        }
    }
    EXPECT_GE(checked, size_t{40});
}
