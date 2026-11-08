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
        std::vector<MODIFICATION> mods;
        for (const nlohmann::json &mod : test_case.at("mods")) {
            mods.push_back(MOD_JSON::parse(mod.dump()));
            EXPECT_TRUE(MOD_CHECKER::check(mods.back()).empty()) << mod.dump();
        }
        uint64_t index = 0;
        for (const nlohmann::json &item : test_case.at("items")) {
            TOKEN_CONTEXT context;
            context.index = index++;
            context.original_name = text(item.at("name").get<std::string>());
            context.parent_name = text(item.value("parent", std::string()));
            context.is_dir = item.value("is_dir", false);
            context.size = item.value("size", int64_t{0});
            std::u32string name = context.original_name;
            for (const MODIFICATION &mod : mods) {
                name = applier_.apply(mod, name, context.is_dir,
                    TOKEN_FORMATTER::build(mod, context));
            }
            EXPECT_EQ(UTF::encode_utf8(name), item.at("expected").get<std::string>())
                << item.at("name").get<std::string>();
            ++checked;
        }
    }
    EXPECT_GE(checked, size_t{40});
}
