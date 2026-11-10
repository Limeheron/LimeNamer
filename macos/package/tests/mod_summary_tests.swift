import Testing

import limenamer_view_model

struct MOD_SUMMARY_TESTS
{
    @Test func loads_cases() throws
    {
        #expect(try !SUMMARY_CASES.load().isEmpty)
    }

    @Test(arguments: try SUMMARY_CASES.load())
    func describes_mods(item: SUMMARY_CASE) throws
    {
        let mod: MOD = try MOD_JSON.parse(item.mod_json)

        #expect(MOD_SUMMARY.describe(mod, direct_index: item.direct_index,
            target_count: item.target_count) == item.expected)
    }

    @Test func describes_mods_read_back_from_engine() throws
    {
        for item in try SUMMARY_CASES.load() {
            let mod: MOD = try MOD_JSON.parse(item.mod_json)
            if mod.direct || mod.scope != .all {
                continue
            }
            let engine = try ENGINE()
            _ = try engine.mod_add(item.mod_json)
            let stored: MOD = try MOD_JSON.parse_list(try engine.mods_get())[0]

            #expect(MOD_SUMMARY.describe(stored, direct_index: 0, target_count: 0)
                == item.expected, "\(item.name)")
        }
    }
}
