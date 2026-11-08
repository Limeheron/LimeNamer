#include "mod_json_test.h"

TEST(MOD_JSON, parses_insert_with_defaults)
{
    const MODIFICATION mod = MOD_JSON::parse(
        R"({"kind":"insert","position":{"type":"start"},"value":[{"text":"2024_"}]})");
    EXPECT_EQ(mod.kind, MOD_KIND::insert);
    EXPECT_EQ(mod.part, MOD_PART::name);
    EXPECT_EQ(mod.position.type, POSITION_TYPE::start);
    ASSERT_EQ(mod.value.size(), size_t{1});
    EXPECT_EQ(std::get<std::u32string>(mod.value[0]), U"2024_");
    EXPECT_EQ(mod.number, NUMBER_SETTINGS());
    EXPECT_EQ(mod.scope, MOD_SCOPE::all);
    EXPECT_TRUE(mod.targets.empty());
    EXPECT_FALSE(mod.direct);
}

TEST(MOD_JSON, parses_text_position_with_options)
{
    const MODIFICATION mod = MOD_JSON::parse(R"({
        "kind": "replace", "part": "full",
        "position": {"type": "text", "text": "IMG_", "case_sensitive": true,
            "occurrence": "nth", "occurrence_n": 2},
        "value": [{"text": "제주_"}, {"token": "number"}],
        "tokens": {"number": {"start": 0, "step": -1, "digits": 3}},
        "scope": "items", "targets": [5, 3, 5], "direct": true})");
    EXPECT_EQ(mod.kind, MOD_KIND::replace);
    EXPECT_EQ(mod.part, MOD_PART::full);
    EXPECT_EQ(mod.position.type, POSITION_TYPE::text);
    EXPECT_EQ(mod.position.text, U"IMG_");
    EXPECT_TRUE(mod.position.case_sensitive);
    EXPECT_EQ(mod.position.occurrence, OCCURRENCE::nth);
    EXPECT_EQ(mod.position.occurrence_n, uint32_t{2});
    const std::vector<VALUE_PIECE> value = {std::u32string(U"제주_"), TOKEN_KIND::number};
    EXPECT_EQ(mod.value, value);
    EXPECT_EQ(mod.number.start, 0);
    EXPECT_EQ(mod.number.step, -1);
    EXPECT_EQ(mod.number.digits, uint32_t{3});
    EXPECT_EQ(mod.scope, MOD_SCOPE::items);
    const std::vector<uint64_t> targets = {3, 5};
    EXPECT_EQ(mod.targets, targets);
    EXPECT_TRUE(mod.direct);
}

TEST(MOD_JSON, parses_numeric_positions)
{
    const MODIFICATION after = MOD_JSON::parse(
        R"({"kind":"insert","position":{"type":"after_nth","n":2},"value":[]})");
    EXPECT_EQ(after.position.type, POSITION_TYPE::after_nth);
    EXPECT_EQ(after.position.n, uint32_t{2});
    const MODIFICATION before = MOD_JSON::parse(
        R"({"kind":"insert","position":{"type":"before_nth_from_end","n":4},"value":[]})");
    EXPECT_EQ(before.position.n, uint32_t{4});
    const MODIFICATION first = MOD_JSON::parse(
        R"({"kind":"delete","position":{"type":"first_n","n":4}})");
    EXPECT_EQ(first.kind, MOD_KIND::remove);
    EXPECT_EQ(first.position.type, POSITION_TYPE::first_n);
    EXPECT_EQ(first.position.n, uint32_t{4});
    const MODIFICATION last = MOD_JSON::parse(
        R"({"kind":"delete","position":{"type":"last_n","n":6}})");
    EXPECT_EQ(last.position.n, uint32_t{6});
    const MODIFICATION range = MOD_JSON::parse(
        R"({"kind":"delete","position":{"type":"from_nth","n":3,"count":2}})");
    EXPECT_EQ(range.position.n, uint32_t{3});
    EXPECT_EQ(range.position.count, uint32_t{2});
}

TEST(MOD_JSON, parses_between_positions)
{
    const MODIFICATION inner = MOD_JSON::parse(R"({"kind":"delete","position":{
        "type":"between","start_text":"[","end_text":"]","include_delimiters":true,
        "occurrence":"last"}})");
    EXPECT_EQ(inner.position.start_text, U"[");
    EXPECT_EQ(inner.position.end_text, U"]");
    EXPECT_FALSE(inner.position.from_beginning);
    EXPECT_FALSE(inner.position.to_end);
    EXPECT_TRUE(inner.position.include_delimiters);
    EXPECT_EQ(inner.position.occurrence, OCCURRENCE::last);
    const MODIFICATION open = MOD_JSON::parse(R"({"kind":"delete","position":{
        "type":"between","from_beginning":true,"to_end":true}})");
    EXPECT_TRUE(open.position.from_beginning);
    EXPECT_TRUE(open.position.to_end);
    EXPECT_TRUE(open.position.start_text.empty());
    EXPECT_TRUE(open.position.end_text.empty());
}

TEST(MOD_JSON, parses_format)
{
    const MODIFICATION upper = MOD_JSON::parse(
        R"({"kind":"format","part":"ext","position":{"type":"all"},"format":{"type":"upper"}})");
    EXPECT_EQ(upper.kind, MOD_KIND::format);
    EXPECT_EQ(upper.part, MOD_PART::ext);
    EXPECT_EQ(upper.format.type, FORMAT_TYPE::upper);
    const MODIFICATION digits = MOD_JSON::parse(R"({"kind":"format","position":{"type":"all"},
        "format":{"type":"pad_digits","digits":3}})");
    EXPECT_EQ(digits.format.type, FORMAT_TYPE::pad_digits);
    EXPECT_EQ(digits.format.digits, uint32_t{3});
}

TEST(MOD_JSON, ignores_fields_that_do_not_apply)
{
    const MODIFICATION mod = MOD_JSON::parse(R"({"kind":"delete",
        "position":{"type":"all","text":"x","n":3},
        "value":[{"text":"x"}], "tokens":{"number":{"start":9}},
        "format":{"type":"lower"}, "scope":"all", "targets":[1,2], "extra":1})");
    POSITION expected_position;
    expected_position.type = POSITION_TYPE::all;
    EXPECT_EQ(mod.position, expected_position);
    EXPECT_TRUE(mod.value.empty());
    EXPECT_EQ(mod.number, NUMBER_SETTINGS());
    EXPECT_EQ(mod.format, FORMAT_SETTINGS());
    EXPECT_TRUE(mod.targets.empty());
}

TEST(MOD_JSON, rejects_invalid_input)
{
    const std::string_view samples[] = {
        "",
        "not json",
        "[]",
        "{}",
        R"({"kind":"move","position":{"type":"start"},"value":[]})",
        R"({"kind":1,"position":{"type":"start"},"value":[]})",
        R"({"kind":"insert","value":[]})",
        R"({"kind":"insert","position":"start","value":[]})",
        R"({"kind":"insert","position":{"type":"middle"},"value":[]})",
        R"({"kind":"insert","position":{"type":"all"},"value":[]})",
        R"({"kind":"delete","position":{"type":"start"}})",
        R"({"kind":"delete","position":{"type":"first_n"}})",
        R"({"kind":"delete","position":{"type":"first_n","n":0}})",
        R"({"kind":"delete","position":{"type":"first_n","n":-1}})",
        R"({"kind":"delete","position":{"type":"first_n","n":1.5}})",
        R"({"kind":"delete","position":{"type":"first_n","n":"2"}})",
        R"({"kind":"delete","position":{"type":"from_nth","n":1}})",
        R"({"kind":"delete","position":{"type":"text"}})",
        R"({"kind":"delete","position":{"type":"text","text":"a","case_sensitive":"yes"}})",
        R"({"kind":"delete","position":{"type":"text","text":"a","occurrence":"second"}})",
        R"({"kind":"delete","position":{"type":"text","text":"a","occurrence":"nth"}})",
        R"({"kind":"delete","position":{"type":"between","start_text":"[","end_text":"]",
            "occurrence":"all"}})",
        R"({"kind":"delete","position":{"type":"between","end_text":"]"}})",
        R"({"kind":"delete","position":{"type":"between","start_text":"["}})",
        R"({"kind":"insert","position":{"type":"start"}})",
        R"({"kind":"insert","position":{"type":"start"},"value":"x"})",
        R"({"kind":"insert","position":{"type":"start"},"value":[{"text":"a","token":"size"}]})",
        R"({"kind":"insert","position":{"type":"start"},"value":[{}]})",
        R"({"kind":"insert","position":{"type":"start"},"value":[{"token":"date"}]})",
        R"({"kind":"insert","position":{"type":"start"},"value":[{"text":1}]})",
        R"({"kind":"insert","position":{"type":"start"},"value":[],
            "tokens":{"number":{"digits":0}}})",
        R"({"kind":"insert","position":{"type":"start"},"value":[],
            "tokens":{"number":{"start":10000000000}}})",
        R"({"kind":"insert","position":{"type":"start"},"value":[],
            "tokens":{"number":{"step":"1"}}})",
        R"({"kind":"format","position":{"type":"all"}})",
        R"({"kind":"format","position":{"type":"all"},"format":{"type":"bold"}})",
        R"({"kind":"format","position":{"type":"all"},"format":{"type":"pad_digits"}})",
        R"({"kind":"format","position":{"type":"all"},"format":{"type":"pad_digits","digits":33}})",
        R"({"kind":"delete","part":"stem","position":{"type":"all"}})",
        R"({"kind":"delete","position":{"type":"all"},"scope":"some"})",
        R"({"kind":"delete","position":{"type":"all"},"scope":"items","targets":1})",
        R"({"kind":"delete","position":{"type":"all"},"scope":"items","targets":[0]})",
        R"({"kind":"delete","position":{"type":"all"},"scope":"items","targets":[-1]})",
        R"({"kind":"delete","position":{"type":"all"},"direct":"true"})",
        "{\"kind\":\"insert\",\"position\":{\"type\":\"start\"},\"value\":[{\"text\":\"\xFF\"}]}",
    };
    for (size_t i = 0; i < std::size(samples); ++i) {
        EXPECT_THROW(MOD_JSON::parse(samples[i]), MOD_JSON_ERROR) << "sample " << i;
    }
}

TEST(MOD_JSON, writes_canonical_json)
{
    const MODIFICATION mod = MOD_JSON::parse(
        R"({"kind":"insert","position":{"type":"start"},"value":[{"text":"2024_"}]})");
    const nlohmann::json expected = nlohmann::json::parse(R"({
        "kind": "insert", "part": "name", "position": {"type": "start"},
        "value": [{"text": "2024_"}], "tokens": {"number": {"start": 1, "step": 1, "digits": 1}},
        "scope": "all", "direct": false})");
    EXPECT_EQ(nlohmann::json::parse(MOD_JSON::to_json(mod)), expected);
}

TEST(MOD_JSON, round_trips)
{
    const std::string_view samples[] = {
        R"({"kind":"insert","position":{"type":"end"},"value":[{"text":"a"}],
            "scope":"items","targets":[3]})",
        R"({"kind":"insert","position":{"type":"before_text","text":"한글 😀",
            "occurrence":"all"},"value":[{"token":"original_name"},{"text":"_"}]})",
        R"({"kind":"insert","position":{"type":"after_text","text":"x",
            "occurrence":"nth_from_end","occurrence_n":3},"value":[{"token":"parent_folder"}]})",
        R"({"kind":"replace","part":"full","position":{"type":"all"},
            "value":[{"text":"abc.jpg"}],"scope":"items","targets":[7],"direct":true})",
        R"({"kind":"replace","position":{"type":"from_nth","n":2,"count":5},
            "value":[{"token":"number"}],"tokens":{"number":{"start":-3,"step":2,"digits":4}}})",
        R"({"kind":"delete","part":"ext","position":{"type":"between","start_text":"(",
            "to_end":true,"case_sensitive":true,"occurrence":"nth","occurrence_n":2}})",
        R"({"kind":"format","position":{"type":"last_n","n":3},
            "format":{"type":"pad_digits","digits":3}})",
        R"({"kind":"format","position":{"type":"text","text":"ab"},"format":{"type":"title"}})",
    };
    for (size_t i = 0; i < std::size(samples); ++i) {
        const MODIFICATION mod = MOD_JSON::parse(samples[i]);
        const std::string json = MOD_JSON::to_json(mod);
        EXPECT_TRUE(MOD_JSON::parse(json) == mod) << "sample " << i << ": " << json;
    }
}

TEST(MOD_JSON, writes_utf8_text)
{
    const MODIFICATION mod = MOD_JSON::parse(
        R"({"kind":"insert","position":{"type":"start"},"value":[{"text":"제주_😀"}]})");
    EXPECT_NE(MOD_JSON::to_json(mod).find("제주_😀"), std::string::npos);
}
