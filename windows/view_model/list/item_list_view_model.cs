using System;
using System.Collections.Generic;
using System.Linq;

namespace limenamer;

public sealed class ITEM_LIST_VIEW_MODEL : OBSERVABLE
{
    private readonly ENGINE engine_;
    private readonly HashSet<ulong> selected_ids_ = new();
    private readonly HashSet<ulong> checked_ids_ = new();
    private ITEM_ROW[] all_rows_ = [];
    private IReadOnlyList<ITEM_ROW> rows_ = [];
    private PREVIEW preview_ = new([], 0, 0, 0, 0, 0);
    private VIEW_FILTER filter_ = VIEW_FILTER.all;
    private bool designating_;
    private ulong check_anchor_id_;
    private string number_text_ = "";
    private string number_error_ = "";

    public ITEM_LIST_VIEW_MODEL(ENGINE engine)
    {
        engine_ = engine;
    }

    public event Action<IReadOnlyCollection<ulong>>? selection_requested;
    public event Action<ulong>? reveal_requested;

    public IReadOnlyList<ITEM_ROW> rows
    {
        get { return rows_; }
        private set { assign(ref rows_, value); }
    }

    public VIEW_FILTER filter
    {
        get { return filter_; }
        set {
            if (assign(ref filter_, value)) {
                apply_filter();
                notify(nameof(filter_label));
            }
        }
    }

    public bool shows_empty_list
    {
        get { return all_rows_.Length == 0; }
    }

    public bool shows_empty_filter
    {
        get { return all_rows_.Length > 0 && rows_.Count == 0; }
    }

    public string empty_filter_title
    {
        get { return JOSA_FORMATTER.format(STRINGS.empty_filter_title, filter_name(filter_)); }
    }

    public string filter_label
    {
        get { return filter_name(filter_); }
    }

    public int count_all
    {
        get { return all_rows_.Length; }
    }

    public uint count_changed
    {
        get { return preview_.changed; }
    }

    public uint count_warning
    {
        get { return preview_.warning; }
    }

    public uint count_error
    {
        get { return preview_.error; }
    }

    public uint count_conflict
    {
        get { return preview_.conflict; }
    }

    public IReadOnlyCollection<ulong> selected_ids
    {
        get { return selected_ids_; }
    }

    public bool can_drag
    {
        get { return filter_ == VIEW_FILTER.all; }
    }

    public string header_text
    {
        get {
            List<string> parts = [JOSA_FORMATTER.format(STRINGS.list_count, all_rows_.Length)];
            if (designating_) {
                parts.Add(JOSA_FORMATTER.format(STRINGS.list_checked, live_checked().Count()));
            } else {
                parts.Add(JOSA_FORMATTER.format(STRINGS.list_selected, selected_ids_.Count));
            }
            if (filter_ != VIEW_FILTER.all) {
                parts.Add(JOSA_FORMATTER.format(STRINGS.list_filtered, filter_name(filter_)));
            }
            return string.Join(STRINGS.list_separator, parts);
        }
    }

    public bool designating
    {
        get { return designating_; }
    }

    public bool? check_all_state
    {
        get {
            int count = live_checked().Count();
            if (count == 0) {
                return false;
            }
            return count == all_rows_.Length ? true : null;
        }
    }

    public IReadOnlyCollection<ulong> checked_ids
    {
        get { return checked_ids_; }
    }

    public string number_text
    {
        get { return number_text_; }
        set { apply_number_text(value); }
    }

    public string number_error
    {
        get { return number_error_; }
        private set { assign(ref number_error_, value); }
    }

    public static string filter_name(VIEW_FILTER filter)
    {
        return filter switch {
            VIEW_FILTER.all => STRINGS.filter_all,
            VIEW_FILTER.changed => STRINGS.filter_changed,
            VIEW_FILTER.warning => STRINGS.filter_warning,
            VIEW_FILTER.error => STRINGS.filter_error,
            _ => STRINGS.filter_conflict,
        };
    }

    public void load(PREVIEW preview)
    {
        preview_ = preview;
        all_rows_ = new ITEM_ROW[preview.rows.Count];
        for (int index = 0; index < all_rows_.Length; index++) {
            ITEM_ROW row = new(preview.rows[index], (uint)index + 1);
            row.is_checked = checked_ids_.Contains(row.item_id);
            row.is_dimmed = designating_ && !row.is_checked;
            all_rows_[index] = row;
        }
        HashSet<ulong> live = all_rows_.Select(row => row.item_id).ToHashSet();
        selected_ids_.IntersectWith(live);
        if (designating_) {
            write_number_text();
        }
        apply_filter();
        notify(nameof(count_all));
        notify(nameof(count_changed));
        notify(nameof(count_warning));
        notify(nameof(count_error));
        notify(nameof(count_conflict));
        notify(nameof(check_all_state));
        notify(nameof(selected_ids));
        selection_requested?.Invoke(selected_ids_);
    }

    public void set_selection(IEnumerable<ulong> ids)
    {
        selected_ids_.Clear();
        selected_ids_.UnionWith(ids);
        notify(nameof(selected_ids));
        notify(nameof(header_text));
    }

    public void clear_selection()
    {
        set_selection([]);
        selection_requested?.Invoke(selected_ids_);
    }

    public void reveal(ulong item_id)
    {
        if (!rows_.Any(row => row.item_id == item_id)) {
            filter = VIEW_FILTER.all;
        }
        set_selection([item_id]);
        selection_requested?.Invoke(selected_ids_);
        reveal_requested?.Invoke(item_id);
    }

    public IReadOnlyList<ulong> drag_ids(ulong grabbed_id)
    {
        if (!selected_ids_.Contains(grabbed_id)) {
            return [grabbed_id];
        }
        return all_rows_.Where(row => selected_ids_.Contains(row.item_id))
            .Select(row => row.item_id).ToArray();
    }

    public bool can_drop(IReadOnlyList<ulong> ids, ulong target_id, bool after)
    {
        if (!can_drag || ids.Contains(target_id)) {
            return false;
        }
        return engine_.items_move(ids.ToArray(), target_id, after, true);
    }

    public bool drop(IReadOnlyList<ulong> ids, ulong target_id, bool after)
    {
        if (!can_drop(ids, target_id, after)) {
            return false;
        }
        return engine_.items_move(ids.ToArray(), target_id, after, false);
    }

    public void begin_designation(IEnumerable<ulong> ids)
    {
        designating_ = true;
        checked_ids_.Clear();
        checked_ids_.UnionWith(ids);
        check_anchor_id_ = 0;
        number_error = "";
        refresh_checks();
        notify(nameof(designating));
    }

    public void end_designation()
    {
        designating_ = false;
        checked_ids_.Clear();
        number_error = "";
        assign(ref number_text_, "", nameof(number_text));
        refresh_checks();
        notify(nameof(designating));
    }

    public void toggle_check(ulong item_id, bool range)
    {
        bool state = !checked_ids_.Contains(item_id);
        List<ulong> ids = [item_id];
        if (range && check_anchor_id_ != 0) {
            int from = index_in(rows_, check_anchor_id_);
            int to = index_in(rows_, item_id);
            if (from >= 0 && to >= 0) {
                ids = rows_.Skip(Math.Min(from, to)).Take(Math.Abs(to - from) + 1)
                    .Select(row => row.item_id).ToList();
            }
        }
        foreach (ulong id in ids) {
            if (state) {
                checked_ids_.Add(id);
            } else {
                checked_ids_.Remove(id);
            }
        }
        check_anchor_id_ = item_id;
        number_error = "";
        refresh_checks();
    }

    public void check_all(bool state)
    {
        checked_ids_.Clear();
        if (state) {
            checked_ids_.UnionWith(all_rows_.Select(row => row.item_id));
        }
        number_error = "";
        refresh_checks();
    }

    public IReadOnlyList<ulong> live_checked_ids()
    {
        return live_checked().Select(row => row.item_id).ToArray();
    }

    public uint index_of(ulong item_id)
    {
        int index = index_in(all_rows_, item_id);
        return index < 0 ? 0 : (uint)index + 1;
    }

    private void apply_number_text(string text)
    {
        assign(ref number_text_, text, nameof(number_text));
        INDEX_LIST list = ENGINE.index_list_parse(text, (uint)all_rows_.Length);
        if (list.error != INDEX_ERROR.none) {
            number_error = MESSAGES.index_error_text(list.error, (uint)all_rows_.Length);
            return;
        }
        number_error = "";
        checked_ids_.ExceptWith(all_rows_.Select(row => row.item_id));
        checked_ids_.UnionWith(list.indices.Select(index => all_rows_[index - 1].item_id));
        update_checks();
    }

    private void refresh_checks()
    {
        update_checks();
        write_number_text();
    }

    private void update_checks()
    {
        foreach (ITEM_ROW row in all_rows_) {
            row.is_checked = checked_ids_.Contains(row.item_id);
            row.is_dimmed = designating_ && !row.is_checked;
        }
        notify(nameof(checked_ids));
        notify(nameof(check_all_state));
        notify(nameof(header_text));
    }

    private void write_number_text()
    {
        uint[] indices = live_checked().Select(row => row.index).ToArray();
        assign(ref number_text_, designating_ ? ENGINE.index_list_format(indices) : "",
            nameof(number_text));
    }

    private IEnumerable<ITEM_ROW> live_checked()
    {
        return all_rows_.Where(row => checked_ids_.Contains(row.item_id));
    }

    private void apply_filter()
    {
        rows = filter_ == VIEW_FILTER.all
            ? all_rows_
            : all_rows_.Where(row => matches(row, filter_)).ToArray();
        notify(nameof(can_drag));
        notify(nameof(header_text));
        notify(nameof(shows_empty_list));
        notify(nameof(shows_empty_filter));
        notify(nameof(empty_filter_title));
    }

    private static bool matches(ITEM_ROW row, VIEW_FILTER filter)
    {
        return filter switch {
            VIEW_FILTER.changed => row.changed,
            VIEW_FILTER.warning => row.status == STATUS.warning,
            VIEW_FILTER.error => row.status == STATUS.error,
            VIEW_FILTER.conflict => row.status == STATUS.conflict,
            _ => true,
        };
    }

    private static int index_in(IReadOnlyList<ITEM_ROW> rows, ulong item_id)
    {
        for (int index = 0; index < rows.Count; index++) {
            if (rows[index].item_id == item_id) {
                return index;
            }
        }
        return -1;
    }
}
