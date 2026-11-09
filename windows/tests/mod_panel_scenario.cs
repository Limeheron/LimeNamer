using System;
using System.Collections.Generic;

namespace limenamer;

internal sealed class MOD_PANEL_SCENARIO : IDisposable
{
    internal const string INSERT_OLD =
        """{"kind":"insert","position":{"type":"start"},"value":[{"text":"old_"}]}""";

    public MOD_PANEL_SCENARIO(params string[] names)
    {
        items = new LIST_SCENARIO(names);
        mods = new MOD_LIST_VIEW_MODEL(items.engine, items.list);
        mods.changed += text => {
            messages.Add(text);
            load();
        };
        mods.notice += text => messages.Add(text);
        mods.editor.focus_requested += field => focused = field;
        load();
    }

    public LIST_SCENARIO items { get; }
    public MOD_LIST_VIEW_MODEL mods { get; }
    public MOD_EDITOR_VIEW_MODEL editor
    {
        get { return mods.editor; }
    }

    public ENGINE engine
    {
        get { return items.engine; }
    }

    public List<string> messages { get; } = new();
    public MOD_FIELD? focused { get; private set; }

    public void load()
    {
        items.load();
        mods.load(engine.preview());
    }

    public string first_new_name()
    {
        return engine.preview().rows[0].new_name;
    }

    public void Dispose()
    {
        items.Dispose();
    }
}
