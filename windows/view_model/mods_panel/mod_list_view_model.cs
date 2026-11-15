using System;
using System.Collections.Generic;
using System.Linq;

namespace limenamer;

public sealed class MOD_LIST_VIEW_MODEL : OBSERVABLE
{
    private static readonly string[] FLOW = [
        nameof(is_existing), nameof(badge_text), nameof(apply_label), nameof(add_back_label),
        nameof(back_label), nameof(edit_kind),
    ];

    private readonly ENGINE engine_;
    private MOD_PAGE page_ = MOD_PAGE.list;
    private IReadOnlyList<MOD_CARD> cards_ = [];
    private MOD_KIND add_kind_ = MOD_KIND.insert;

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

    public bool has_mods
    {
        get { return cards_.Count > 0; }
    }

    public bool has_no_mods
    {
        get { return cards_.Count == 0; }
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

    public static IReadOnlyList<CHOICE<MOD_KIND>> kind_choices { get; } = [
        new(MOD_KIND.insert, STRINGS.kind_insert),
        new(MOD_KIND.delete, STRINGS.kind_delete),
        new(MOD_KIND.replace, STRINGS.kind_replace),
        new(MOD_KIND.format, STRINGS.kind_format),
    ];

    public MOD_KIND add_kind
    {
        get { return add_kind_; }
        set {
            if (assign(ref add_kind_, value)) {
                notify(nameof(kind_header));
                notify(nameof(position_cards));
            }
        }
    }

    public KIND_CARD kind_header
    {
        get { return kind_card(add_kind_); }
    }

    public IReadOnlyList<POSITION_CHOICE> position_cards
    {
        get { return editor.position_choices(add_kind_); }
    }

    public MOD_KIND edit_kind
    {
        get { return editor.kind; }
        set { switch_kind(value); }
    }

    public bool is_existing
    {
        get { return editor.is_active && !editor.is_new; }
    }

    public string badge_text
    {
        get { return is_existing ? STRINGS.badge_editing : STRINGS.badge_new; }
    }

    public string apply_label
    {
        get { return is_existing ? STRINGS.button_apply : STRINGS.button_add; }
    }

    public string add_back_label
    {
        get { return is_existing ? STRINGS.back_to_edit : STRINGS.back_to_list; }
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
        notify(nameof(has_mods));
        notify(nameof(has_no_mods));
        notify(nameof(all_enabled_state));
        notify(nameof(enabled_count));
        if (page_ == MOD_PAGE.edit) {
            editor.refresh();
        }
    }

    public void open_add()
    {
        add_kind = MOD_KIND.insert;
        show(MOD_PAGE.add);
    }

    public void choose_position(POSITION_TYPE position)
    {
        if (is_existing) {
            editor.restart(add_kind_, position);
        } else {
            editor.begin_new(add_kind_, position);
        }
        show(MOD_PAGE.edit);
    }

    public void switch_kind(MOD_KIND kind)
    {
        if (page_ != MOD_PAGE.edit || kind == editor.kind) {
            return;
        }
        if (editor.is_new) {
            editor.cancel();
        }
        add_kind = kind;
        show(MOD_PAGE.add);
    }

    public void open_card(MOD_CARD card)
    {
        editor.begin_edit(card.mod);
        show(MOD_PAGE.edit);
    }

    public bool apply()
    {
        bool was_new = editor.is_new;
        if (!editor.apply()) {
            return false;
        }
        show(MOD_PAGE.list);
        changed?.Invoke(was_new ? STRINGS.mod_added : STRINGS.mod_updated);
        return true;
    }

    public string back_label
    {
        get { return editor.is_new ? STRINGS.back_to_add : STRINGS.back_to_list; }
    }

    public void go_back()
    {
        if (page_ == MOD_PAGE.edit && editor.is_new) {
            MOD_KIND kind = editor.kind;
            editor.cancel();
            add_kind = kind;
            show(MOD_PAGE.add);
            return;
        }
        if (page_ == MOD_PAGE.edit) {
            cancel();
            return;
        }
        show(is_existing ? MOD_PAGE.edit : MOD_PAGE.list);
    }

    public void cancel()
    {
        string text = editor.cancel();
        show(MOD_PAGE.list);
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

    private void show(MOD_PAGE target)
    {
        page = target;
        foreach (string name in FLOW) {
            notify(name);
        }
    }

    private static KIND_CARD kind_card(MOD_KIND kind)
    {
        string key = $"example_name_{kind}";
        return new KIND_CARD(kind, text($"kind_{kind}"), text($"kind_{kind}_desc"),
            text(key + "_before"), text(key + "_after"));
    }

    private static string text(string key)
    {
        return STRINGS.ResourceManager.GetString(key, STRINGS.Culture) ?? key;
    }
}
