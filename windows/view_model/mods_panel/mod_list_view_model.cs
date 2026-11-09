using System;
using System.Collections.Generic;
using System.Linq;

namespace limenamer;

public sealed class MOD_LIST_VIEW_MODEL : OBSERVABLE
{
    private static readonly MOD_KIND[] KINDS =
        [MOD_KIND.insert, MOD_KIND.delete, MOD_KIND.replace, MOD_KIND.format];

    private readonly ENGINE engine_;
    private MOD_PAGE page_ = MOD_PAGE.list;
    private IReadOnlyList<MOD_CARD> cards_ = [];
    private PART add_part_ = PART.name;
    private bool panel_open_ = true;

    public MOD_LIST_VIEW_MODEL(ENGINE engine, ITEM_LIST_VIEW_MODEL list)
    {
        engine_ = engine;
        editor = new MOD_EDITOR_VIEW_MODEL(engine, list);
        editor.notice += text => notice?.Invoke(text);
    }

    public event Action<string>? changed;
    public event Action<string>? notice;

    public MOD_EDITOR_VIEW_MODEL editor { get; }

    public MOD_PAGE page
    {
        get { return page_; }
        private set {
            if (assign(ref page_, value)) {
                notify(nameof(is_editing));
            }
        }
    }

    public bool is_editing
    {
        get { return page_ == MOD_PAGE.edit; }
    }

    public IReadOnlyList<MOD_CARD> cards
    {
        get { return cards_; }
        private set { assign(ref cards_, value); }
    }

    public bool? all_enabled_state
    {
        get {
            int enabled = enabled_count;
            if (enabled == 0) {
                return false;
            }
            return enabled == cards_.Count ? true : null;
        }
    }

    public int enabled_count
    {
        get { return cards_.Count(card => card.enabled); }
    }

    public bool panel_open
    {
        get { return panel_open_; }
        set { assign(ref panel_open_, value); }
    }

    public PART add_part
    {
        get { return add_part_; }
        set {
            if (assign(ref add_part_, value)) {
                notify(nameof(kind_cards));
            }
        }
    }

    public IReadOnlyList<KIND_CARD> kind_cards
    {
        get { return KINDS.Select(kind => kind_card(kind, add_part_)).ToArray(); }
    }

    public void load(PREVIEW preview)
    {
        Dictionary<ulong, uint> positions = new();
        for (int index = 0; index < preview.rows.Count; index++) {
            positions[preview.rows[index].item_id] = (uint)index + 1;
        }

        List<MOD_CARD> cards = new();
        foreach (MOD mod in MOD_JSON.parse_list(engine_.mods_get())) {
            IReadOnlyList<ulong> targets = mod.targets ?? [];
            uint live = (uint)targets.Count(positions.ContainsKey);
            uint direct_index = 0;
            if (mod.direct && targets.Count > 0) {
                positions.TryGetValue(targets[0], out direct_index);
            }
            cards.Add(new MOD_CARD(mod.id ?? 0, mod.enabled ?? true, mod.direct,
                MOD_SUMMARY.describe(mod, direct_index, live), mod));
        }
        this.cards = cards;
        notify(nameof(all_enabled_state));
        notify(nameof(enabled_count));
        if (page_ == MOD_PAGE.edit) {
            editor.refresh();
        }
    }

    public void open_add()
    {
        add_part = PART.name;
        page = MOD_PAGE.add;
    }

    public void close_add()
    {
        page = MOD_PAGE.list;
    }

    public void choose_kind(MOD_KIND kind)
    {
        editor.begin_new(kind, add_part_);
        page = MOD_PAGE.edit;
    }

    public void open_card(MOD_CARD card)
    {
        editor.begin_edit(card.mod);
        page = MOD_PAGE.edit;
    }

    public bool apply()
    {
        bool was_new = editor.is_new;
        if (!editor.apply()) {
            return false;
        }
        page = MOD_PAGE.list;
        changed?.Invoke(was_new ? STRINGS.mod_added : STRINGS.mod_updated);
        return true;
    }

    public void cancel()
    {
        string text = editor.cancel();
        page = MOD_PAGE.list;
        if (text != "") {
            notice?.Invoke(text);
        }
    }

    public void set_enabled(MOD_CARD card, bool enabled)
    {
        engine_.mod_set_enabled(card.id, enabled);
        changed?.Invoke(enabled ? STRINGS.mod_enabled : STRINGS.mod_disabled);
    }

    public void toggle_all()
    {
        bool enable = all_enabled_state != true;
        engine_.mods_set_all_enabled(enable);
        changed?.Invoke(enable ? STRINGS.mods_all_enabled : STRINGS.mods_all_disabled);
    }

    public void remove(MOD_CARD card)
    {
        engine_.mod_remove(card.id);
        changed?.Invoke(STRINGS.mod_removed);
    }

    public bool can_drop(ulong id, ulong target_id, bool after)
    {
        return id != target_id && engine_.mod_move(id, target_id, after, true);
    }

    public bool drop(ulong id, ulong target_id, bool after)
    {
        if (!can_drop(id, target_id, after)) {
            return false;
        }
        engine_.mod_move(id, target_id, after, false);
        changed?.Invoke(STRINGS.mod_moved);
        return true;
    }

    private static KIND_CARD kind_card(MOD_KIND kind, PART part)
    {
        string key = $"example_{part}_{kind}";
        return new KIND_CARD(kind, text($"kind_{kind}"), text($"kind_{kind}_desc"),
            text(key + "_before"), text(key + "_after"));
    }

    private static string text(string key)
    {
        return STRINGS.ResourceManager.GetString(key, STRINGS.Culture) ?? key;
    }
}
