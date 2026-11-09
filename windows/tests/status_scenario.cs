namespace limenamer;

internal static class STATUS_SCENARIO
{
    internal static LIST_SCENARIO create()
    {
        LIST_SCENARIO scenario = new("a.txt", "b.txt", "c.txt", "d.txt", "e.txt");
        scenario.temp.file("new_b.txt");
        scenario.engine.folders_refresh();
        scenario.engine.mod_add(LIST_SCENARIO.INSERT_NEW);
        scenario.engine.mod_direct_edit(scenario.id("c.txt"), "c?.txt");
        scenario.engine.mod_direct_edit(scenario.id("d.txt"), ".d");
        scenario.engine.mod_direct_edit(scenario.id("e.txt"), "e.txt");
        scenario.load();
        return scenario;
    }
}
