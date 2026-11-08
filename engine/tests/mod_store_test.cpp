#include "mod_store_test.h"

TEST_F(MOD_STORE_TEST, adds_enabled_modifications_in_order)
{
    const uint64_t first = add(INSERT_A);
    const uint64_t second = add(INSERT_B);
    EXPECT_LT(first, second);
    EXPECT_EQ(ids(), std::vector<uint64_t>({first, second}));
    EXPECT_TRUE(store_.find(first)->enabled);
    EXPECT_EQ(store_.find(999), nullptr);
}

TEST_F(MOD_STORE_TEST, stores_texts_in_nfc)
{
    const uint64_t id = add(R"({"kind":"replace","position":{"type":"text","text":"é"},
        "value":[{"text":"café"},{"token":"number"}]})");
    const MODIFICATION &mod = store_.find(id)->mod;
    EXPECT_EQ(mod.position.text, U"é");
    EXPECT_EQ(std::get<std::u32string>(mod.value[0]), U"café");
    const uint64_t between = add(R"({"kind":"delete","position":{"type":"between",
        "start_text":"ä","end_text":"ö"}})");
    EXPECT_EQ(store_.find(between)->mod.position.start_text, U"ä");
    EXPECT_EQ(store_.find(between)->mod.position.end_text, U"ö");
}

TEST_F(MOD_STORE_TEST, updates_a_modification_in_place)
{
    const uint64_t first = add(INSERT_A);
    const uint64_t second = add(INSERT_B);
    store_.set_enabled(first, false);
    EXPECT_TRUE(store_.update(first, MOD_JSON::parse(
        R"({"kind":"insert","position":{"type":"end"},"value":[{"text":"z"}]})")));
    EXPECT_EQ(ids(), std::vector<uint64_t>({first, second}));
    EXPECT_FALSE(store_.find(first)->enabled);
    EXPECT_EQ(store_.find(first)->mod.position.type, POSITION_TYPE::end);
    EXPECT_FALSE(store_.update(999, MOD_JSON::parse(INSERT_A)));
}

TEST_F(MOD_STORE_TEST, keeps_the_kind_when_updating)
{
    const uint64_t id = add(INSERT_A);
    EXPECT_THROW(store_.update(id, MOD_JSON::parse(DELETE_ALL)), std::invalid_argument);
    EXPECT_EQ(store_.find(id)->mod.kind, MOD_KIND::insert);
}

TEST_F(MOD_STORE_TEST, removes_modifications)
{
    const uint64_t first = add(INSERT_A);
    const uint64_t second = add(INSERT_B);
    EXPECT_TRUE(store_.remove(first));
    EXPECT_FALSE(store_.remove(first));
    EXPECT_EQ(ids(), std::vector<uint64_t>({second}));
}

TEST_F(MOD_STORE_TEST, moves_a_modification)
{
    const uint64_t a = add(INSERT_A);
    const uint64_t b = add(INSERT_B);
    const uint64_t c = add(DELETE_ALL);
    EXPECT_EQ(store_.move(a, c, true, false), true);
    EXPECT_EQ(ids(), std::vector<uint64_t>({b, c, a}));
    EXPECT_EQ(store_.move(a, b, false, false), true);
    EXPECT_EQ(ids(), std::vector<uint64_t>({a, b, c}));
}

TEST_F(MOD_STORE_TEST, reports_moves_that_change_nothing)
{
    const uint64_t a = add(INSERT_A);
    const uint64_t b = add(INSERT_B);
    const uint64_t c = add(DELETE_ALL);
    EXPECT_EQ(store_.move(b, a, true, true), false);
    EXPECT_EQ(store_.move(b, c, false, true), false);
    EXPECT_EQ(store_.move(b, b, true, true), false);
    EXPECT_EQ(store_.move(b, c, true, true), true);
    EXPECT_EQ(ids(), std::vector<uint64_t>({a, b, c}));
    EXPECT_FALSE(store_.move(999, a, true, false).has_value());
    EXPECT_FALSE(store_.move(a, 999, true, false).has_value());
}

TEST_F(MOD_STORE_TEST, turns_modifications_on_and_off)
{
    const uint64_t a = add(INSERT_A);
    const uint64_t b = add(INSERT_B);
    EXPECT_TRUE(store_.set_enabled(a, false));
    EXPECT_FALSE(store_.find(a)->enabled);
    EXPECT_TRUE(store_.find(b)->enabled);
    EXPECT_FALSE(store_.set_enabled(999, false));
    store_.set_all_enabled(false);
    EXPECT_FALSE(store_.find(b)->enabled);
    store_.set_all_enabled(true);
    EXPECT_TRUE(store_.find(a)->enabled);
}

TEST_F(MOD_STORE_TEST, adds_a_direct_edit_at_the_end)
{
    const uint64_t a = add(INSERT_A);
    const uint64_t direct = store_.direct_edit(42, U"abc.jpg");
    EXPECT_EQ(ids(), std::vector<uint64_t>({a, direct}));
    const MODIFICATION &mod = store_.find(direct)->mod;
    EXPECT_EQ(mod.kind, MOD_KIND::replace);
    EXPECT_EQ(mod.part, MOD_PART::full);
    EXPECT_EQ(mod.position.type, POSITION_TYPE::all);
    EXPECT_EQ(mod.scope, MOD_SCOPE::items);
    EXPECT_EQ(mod.targets, std::vector<uint64_t>({42}));
    EXPECT_TRUE(mod.direct);
    EXPECT_EQ(mod.value, std::vector<VALUE_PIECE>({std::u32string(U"abc.jpg")}));
}

TEST_F(MOD_STORE_TEST, reuses_the_direct_edit_of_an_item)
{
    const uint64_t first = store_.direct_edit(42, U"one.jpg");
    const uint64_t other = store_.direct_edit(7, U"seven.jpg");
    const uint64_t later = add(INSERT_A);
    store_.set_enabled(first, false);
    EXPECT_EQ(store_.direct_edit(42, U"two.jpg"), first);
    EXPECT_EQ(ids(), std::vector<uint64_t>({first, other, later}));
    EXPECT_TRUE(store_.find(first)->enabled);
    EXPECT_EQ(store_.find(first)->mod.value,
        std::vector<VALUE_PIECE>({std::u32string(U"two.jpg")}));
    EXPECT_EQ(store_.find(other)->mod.value,
        std::vector<VALUE_PIECE>({std::u32string(U"seven.jpg")}));
}

TEST_F(MOD_STORE_TEST, keeps_the_direct_flag_when_updating)
{
    const uint64_t direct = store_.direct_edit(42, U"one.jpg");
    MODIFICATION edited = store_.find(direct)->mod;
    edited.direct = false;
    edited.value = {std::u32string(U"edited.jpg")};
    EXPECT_TRUE(store_.update(direct, edited));
    EXPECT_TRUE(store_.find(direct)->mod.direct);
}

TEST_F(MOD_STORE_TEST, restores_earlier_entries)
{
    const uint64_t a = add(INSERT_A);
    const std::vector<MOD_ENTRY> saved = store_.entries();
    const uint64_t b = add(INSERT_B);
    store_.remove(a);
    store_.restore(saved);
    EXPECT_EQ(ids(), std::vector<uint64_t>({a}));
    const uint64_t c = add(DELETE_ALL);
    EXPECT_GT(c, b);
    std::vector<MOD_ENTRY> unknown = store_.entries();
    unknown[0].id = 999;
    EXPECT_THROW(store_.restore(unknown), std::invalid_argument);
}
