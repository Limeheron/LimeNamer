using System;
using System.Linq;

namespace limenamer;

internal sealed class LIST_SCENARIO : IDisposable
{
    internal const string INSERT_NEW =
        """{"kind":"insert","position":{"type":"start"},"value":[{"text":"new_"}]}""";

    public LIST_SCENARIO(params string[] names)
    {
        temp = new TEMP_FOLDER();
        engine = new ENGINE();
        list = new ITEM_LIST_VIEW_MODEL(engine);
        engine.items_add(names.Select(name => temp.file(name)).ToArray(), ADD_MODE.as_items);
        load();
    }

    public TEMP_FOLDER temp { get; }
    public ENGINE engine { get; }
    public ITEM_LIST_VIEW_MODEL list { get; }

    public ulong id(string name)
    {
        return engine.preview().rows.First(row => row.name == name).item_id;
    }

    public void load()
    {
        list.load(engine.preview());
    }

    public string[] visible_names()
    {
        return list.rows.Select(row => row.name).ToArray();
    }

    public void Dispose()
    {
        engine.Dispose();
        temp.Dispose();
    }
}
