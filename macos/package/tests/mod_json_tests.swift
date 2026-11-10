import Testing

import limenamer_view_model

struct MOD_JSON_TESTS
{
    @Test func writes_back_what_the_engine_stores() throws
    {
        for item in try SUMMARY_CASES.load() {
            if try MOD_JSON.parse(item.mod_json).direct {
                continue
            }
            let engine = try ENGINE()
            _ = try engine.mod_add(item.mod_json)
            let before: String = try engine.mods_get()
            let stored: MOD = try MOD_JSON.parse_list(before)[0]
            try engine.mod_update(try #require(stored.id), json: MOD_JSON.write(stored))

            #expect(try engine.mods_get() == before, "\(item.name)")
        }
    }

    @Test func reads_engine_fields() throws
    {
        let engine = try ENGINE()
        _ = try engine.mod_add("""
            {"kind": "insert", "position": {"type": "end"},
                "value": [{"text": "_"}, {"token": "number"}],
                "tokens": {"number": {"start": 5, "step": 2, "digits": 3}}}
            """)

        let mods: [MOD] = try MOD_JSON.parse_list(try engine.mods_get())

        try #require(mods.count == 1)
        let mod: MOD = mods[0]
        #expect(mod.enabled == true)
        #expect(mod.kind == .insert)
        #expect(mod.part == .name)
        #expect(mod.position.type == .end)
        #expect(mod.value?[0].text == "_")
        #expect(mod.value?[1].token == .number)
        #expect(mod.tokens?.number == NUMBER_SETTINGS(start: 5, step: 2, digits: 3))
        #expect(mod.tokens?.modified_date?.format == "yyyyMMdd")
        #expect(mod.tokens?.size?.unit == .auto)
        #expect(mod.scope == .all)
        #expect(!mod.direct)
    }

    @Test func writes_options_the_engine_reads() throws
    {
        let engine = try ENGINE()
        let options = ENGINE_OPTIONS(duplicate: DUPLICATE_OPTIONS(enabled: true,
            format: .custom, custom_text: "-copy", mode: .number_all))

        try engine.options_set(MOD_JSON.write_options(options))

        #expect(try MOD_JSON.parse_options(try engine.options_get()) == options)
    }
}
