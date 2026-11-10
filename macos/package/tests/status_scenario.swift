import limenamer_view_model

@MainActor
enum STATUS_SCENARIO
{
    static func create() throws -> LIST_SCENARIO
    {
        let scenario = try LIST_SCENARIO("a.txt", "b.txt", "c.txt", "d.txt", "e.txt")
        _ = try scenario.temp.file("new_b.txt")
        try scenario.engine.folders_refresh()
        _ = try scenario.engine.mod_add(TEST_MODS.INSERT_NEW)
        try scenario.engine.mod_direct_edit(try scenario.id("c.txt"), new_name: "c:.txt")
        try scenario.engine.mod_direct_edit(try scenario.id("d.txt"), new_name: ".d")
        try scenario.engine.mod_direct_edit(try scenario.id("e.txt"), new_name: "e.txt")
        try scenario.load()
        return scenario
    }
}
