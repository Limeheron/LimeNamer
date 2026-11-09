using System;
using System.IO;
using System.Linq;

namespace limenamer;

internal sealed class MAIN_SCENARIO : IDisposable
{
    public MAIN_SCENARIO(params string[] names)
    {
        temp = new TEMP_FOLDER();
        engine = new ENGINE();
        settings = SETTINGS.load(Path.Combine(temp.path, "settings", "settings.json"));
        main = new MAIN_VIEW_MODEL(engine, settings, dialogs,
            () => new DateTime(2026, 11, 9, 15, 42, 0));
        if (names.Length > 0) {
            main.add_paths(names.Select(name => temp.file(name)).ToArray(), ADD_MODE.as_items);
        }
    }

    public TEMP_FOLDER temp { get; }
    public ENGINE engine { get; }
    public SETTINGS settings { get; }
    public FAKE_DIALOGS dialogs { get; } = new();
    public MAIN_VIEW_MODEL main { get; }

    public string message
    {
        get { return main.status.message; }
    }

    public ulong id(string name)
    {
        return main.list.rows.First(row => row.name == name).item_id;
    }

    public void add_mod(string json)
    {
        engine.mod_add(json);
        main.refresh();
    }

    public void Dispose()
    {
        engine.Dispose();
        temp.Dispose();
    }
}
