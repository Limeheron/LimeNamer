using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Threading.Tasks;

namespace limenamer;

public sealed class MAIN_VIEW_MODEL : OBSERVABLE
{
    private const int CUSTOM_TEXT_LIMIT = 32;

    private readonly ENGINE engine_;
    private readonly DIALOG_SERVICE dialogs_;
    private readonly COMMAND[] commands_;
    private PREVIEW preview_ = new([], 0, 0, 0, 0, 0);
    private bool running_;
    private bool can_undo_;
    private bool can_redo_;
    private bool undo_available_;
    private SORT_KEY sort_key_;
    private bool sort_descending_;
    private DUPLICATE_OPTIONS duplicate_ = new();
    private string duplicate_custom_text_ = "_copy";

    public MAIN_VIEW_MODEL(ENGINE engine, SETTINGS settings, DIALOG_SERVICE dialogs,
        Func<DateTime> now)
    {
        engine_ = engine;
        dialogs_ = dialogs;
        this.settings = settings;
        list = new ITEM_LIST_VIEW_MODEL(engine);
        mods = new MOD_LIST_VIEW_MODEL(engine, list);
        status = new STATUS_BAR_VIEW_MODEL(list, now);
        mods.changed += text => {
            refresh();
            status.post(text);
        };
        mods.notice += status.post;
        mods.PropertyChanged += on_mods_changed;
        list.PropertyChanged += on_list_changed;

        remove_command = new COMMAND(_ => remove_selected(),
            _ => !running_ && list.selected_ids.Count > 0);
        clear_command = new COMMAND(_ => clear(), _ => !running_ && list.count_all > 0);
        undo_command = new COMMAND(_ => undo(), _ => !running_ && can_undo_);
        redo_command = new COMMAND(_ => redo(), _ => !running_ && can_redo_);
        rename_command = new COMMAND(_ => fire(rename), _ => !running_ && preview_.changed > 0);
        rename_undo_command = new COMMAND(_ => fire(rename_undo),
            _ => !running_ && undo_available_);
        toggle_panel_command = new COMMAND(_ => panel_open = !panel_open, _ => !running_);
        commands_ = [remove_command, clear_command, undo_command, redo_command, rename_command,
            rename_undo_command, toggle_panel_command];
        refresh();
    }

    public ITEM_LIST_VIEW_MODEL list { get; }
    public MOD_LIST_VIEW_MODEL mods { get; }
    public STATUS_BAR_VIEW_MODEL status { get; }
    public SETTINGS settings { get; }
    public COMMAND remove_command { get; }
    public COMMAND clear_command { get; }
    public COMMAND undo_command { get; }
    public COMMAND redo_command { get; }
    public COMMAND rename_command { get; }
    public COMMAND rename_undo_command { get; }
    public COMMAND toggle_panel_command { get; }

    public static IReadOnlyList<CHOICE<SORT_KEY>> sort_choices { get; } = [
        new(SORT_KEY.added, STRINGS.sort_added),
        new(SORT_KEY.name_natural, STRINGS.sort_name_natural),
        new(SORT_KEY.name_ordinal, STRINGS.sort_name_ordinal),
        new(SORT_KEY.modified, STRINGS.sort_modified),
    ];

    public static IReadOnlyList<CHOICE<DUPLICATE_FORMAT>> duplicate_format_choices { get; } = [
        new(DUPLICATE_FORMAT.paren, STRINGS.duplicate_paren),
        new(DUPLICATE_FORMAT.underscore, STRINGS.duplicate_underscore),
        new(DUPLICATE_FORMAT.dash, STRINGS.duplicate_dash),
        new(DUPLICATE_FORMAT.custom, STRINGS.duplicate_custom),
    ];

    public static IReadOnlyList<CHOICE<DUPLICATE_MODE>> duplicate_mode_choices { get; } = [
        new(DUPLICATE_MODE.keep_first, STRINGS.duplicate_keep_first),
        new(DUPLICATE_MODE.number_all, STRINGS.duplicate_number_all),
    ];

    public static IReadOnlyList<CHOICE<THEME_MODE>> theme_choices { get; } = [
        new(THEME_MODE.system, STRINGS.theme_system),
        new(THEME_MODE.light, STRINGS.theme_light),
        new(THEME_MODE.dark, STRINGS.theme_dark),
    ];

    public bool is_running
    {
        get { return running_; }
    }

    public bool can_edit
    {
        get { return !running_; }
    }

    public bool is_editing
    {
        get { return mods.is_editing; }
    }

    public SORT_KEY sort_key
    {
        get { return sort_key_; }
    }

    public bool sort_descending
    {
        get { return sort_descending_; }
    }

    public bool panel_open
    {
        get { return settings.panel_open; }
        set {
            if (!value && mods.is_editing) {
                status.post(JOSA_FORMATTER.format(STRINGS.blocked_panel, STRINGS.edit_block));
                notify(nameof(panel_open));
                return;
            }
            settings.panel_open = value;
            notify(nameof(panel_open));
        }
    }

    public bool duplicate_enabled
    {
        get { return duplicate_.enabled; }
        set { apply_duplicate(duplicate_ with { enabled = value }); }
    }

    public DUPLICATE_FORMAT duplicate_format
    {
        get { return duplicate_.format; }
        set { apply_duplicate(duplicate_ with { format = value }); }
    }

    public DUPLICATE_MODE duplicate_mode
    {
        get { return duplicate_.mode; }
        set { apply_duplicate(duplicate_ with { mode = value }); }
    }

    public string duplicate_custom_text
    {
        get { return duplicate_custom_text_; }
        set {
            assign(ref duplicate_custom_text_, value);
            apply_duplicate(duplicate_);
        }
    }

    public void refresh()
    {
        preview_ = engine_.preview();
        list.load(preview_);
        mods.load(preview_);
        (can_undo_, can_redo_) = engine_.history_state();
        undo_available_ = engine_.rename_undo_state().available;
        (sort_key_, sort_descending_) = engine_.items_sort_state();
        duplicate_ = MOD_JSON.parse_options(engine_.options_get()).duplicate;
        if (duplicate_.custom_text != null) {
            duplicate_custom_text_ = duplicate_.custom_text;
        }
        notify(nameof(sort_key));
        notify(nameof(sort_descending));
        notify(nameof(duplicate_enabled));
        notify(nameof(duplicate_format));
        notify(nameof(duplicate_mode));
        notify(nameof(duplicate_custom_text));
        refresh_commands();
    }

    public void add_paths(IReadOnlyList<string> paths, ADD_MODE mode)
    {
        if (running_ || paths.Count == 0) {
            return;
        }
        ADD_RESULT result = default;
        if (!attempt(() => result = engine_.items_add(paths, mode))) {
            return;
        }
        refresh();
        string text = JOSA_FORMATTER.format(STRINGS.items_added, result.added);
        if (result.skipped > 0) {
            text += JOSA_FORMATTER.format(STRINGS.items_added_skipped, result.skipped);
        }
        if (result.failed > 0) {
            text += JOSA_FORMATTER.format(STRINGS.items_added_failed, result.failed);
        }
        status.post(text);
    }

    public void remove_selected()
    {
        ulong[] ids = list.selected_ids.ToArray();
        if (running_ || ids.Length == 0 || !attempt(() => engine_.items_remove(ids))) {
            return;
        }
        refresh();
        status.post(JOSA_FORMATTER.format(STRINGS.items_removed, ids.Length));
    }

    public void clear()
    {
        if (running_ || !attempt(engine_.items_clear)) {
            return;
        }
        refresh();
        status.post(STRINGS.items_cleared);
    }

    public void undo()
    {
        if (running_ || !can_undo_) {
            return;
        }
        if (mods.is_editing) {
            status.post(JOSA_FORMATTER.format(STRINGS.blocked_undo, STRINGS.edit_block));
            return;
        }
        if (attempt(engine_.undo)) {
            refresh();
            status.post(STRINGS.undone);
        }
    }

    public void redo()
    {
        if (running_ || !can_redo_) {
            return;
        }
        if (mods.is_editing) {
            status.post(JOSA_FORMATTER.format(STRINGS.blocked_redo, STRINGS.edit_block));
            return;
        }
        if (attempt(engine_.redo)) {
            refresh();
            status.post(STRINGS.redone);
        }
    }

    public void sort(SORT_KEY key, bool descending)
    {
        if (!running_ && attempt(() => engine_.items_sort(key, descending))) {
            refresh();
        }
    }

    public void move_rows(IReadOnlyList<ulong> ids, ulong target_id, bool after)
    {
        bool moved = false;
        if (running_ || !attempt(() => moved = list.drop(ids, target_id, after)) || !moved) {
            return;
        }
        refresh();
        status.post(ids.Count > 1
            ? JOSA_FORMATTER.format(STRINGS.items_moved_many, ids.Count)
            : STRINGS.items_moved);
    }

    public void direct_edit(ulong item_id, string new_name)
    {
        if (running_) {
            return;
        }
        if (mods.is_editing) {
            status.post(STRINGS.blocked_direct);
            return;
        }
        if (attempt(() => engine_.mod_direct_edit(item_id, new_name))) {
            refresh();
            status.post(JOSA_FORMATTER.format(STRINGS.direct_edited, list.index_of(item_id)));
        }
    }

    public void show_row_problems(ulong item_id)
    {
        ITEM_ROW? row = list.rows.FirstOrDefault(item => item.item_id == item_id);
        if (row != null && row.problems.Count > 0) {
            status.post(JOSA_FORMATTER.format(STRINGS.row_problems, row.index,
                string.Join(" ", row.problems)));
        }
    }

    public string? location_of(ulong item_id)
    {
        ITEM_ROW? row = list.rows.FirstOrDefault(item => item.item_id == item_id);
        if (row == null || !Path.Exists(row.full_path)) {
            status.post(STRINGS.location_missing);
            return null;
        }
        return row.full_path;
    }

    public void activated()
    {
        if (!running_ && attempt(engine_.folders_refresh)) {
            refresh();
        }
    }

    public void set_theme(THEME_MODE mode)
    {
        settings.theme_mode = mode;
        status.post(JOSA_FORMATTER.format(STRINGS.theme_changed, theme_name(mode)));
    }

    public void toggle_theme(bool system_dark)
    {
        bool dark = settings.theme_mode == THEME_MODE.dark
            || (settings.theme_mode == THEME_MODE.system && system_dark);
        set_theme(dark ? THEME_MODE.light : THEME_MODE.dark);
    }

    public async Task rename()
    {
        if (running_) {
            return;
        }
        if (mods.is_editing) {
            status.post(JOSA_FORMATTER.format(STRINGS.blocked_rename, STRINGS.edit_block));
            return;
        }
        if (preview_.error + preview_.conflict > 0) {
            report_blocked();
            return;
        }
        if (preview_.changed == 0) {
            status.post(STRINGS.rename_nothing);
            return;
        }
        if (preview_.warning > 0 && !dialogs_.ask(warning_dialog())) {
            return;
        }

        REPORT report;
        try {
            report = await run_engine(STRINGS.progress_renaming, engine_.rename);
        } catch (ENGINE_EXCEPTION error) when (error.result == RESULT.not_ready) {
            refresh();
            status.post(STRINGS.rename_not_ready);
            reveal_first_problem();
            return;
        } catch (ENGINE_EXCEPTION error) {
            refresh();
            status.post(MESSAGES.result_text(error.result));
            return;
        }

        if (report.needs_choice) {
            await resolve(report);
            return;
        }
        refresh();
        if (report.failed > 0) {
            dialogs_.ask(failed_dialog(report));
        }
        if (report.succeeded == 0) {
            status.post(JOSA_FORMATTER.format(STRINGS.rename_failed_all, report.failed));
            return;
        }
        finish_rename(report.failed > 0
            ? JOSA_FORMATTER.format(STRINGS.renamed_partly, report.succeeded, report.failed)
            : JOSA_FORMATTER.format(STRINGS.renamed, report.succeeded));
    }

    public async Task rename_undo()
    {
        if (running_ || !undo_available_) {
            return;
        }
        if (mods.is_editing) {
            status.post(JOSA_FORMATTER.format(STRINGS.blocked_rename_undo, STRINGS.edit_block));
            return;
        }
        REPORT check = engine_.rename_undo_check();
        uint count = check.total - check.skipped;
        if (count == 0) {
            status.post(STRINGS.undo_nothing);
            return;
        }
        if (!dialogs_.ask(undo_dialog(check, count))) {
            return;
        }

        REPORT report;
        try {
            report = await run_engine(STRINGS.progress_reverting, engine_.rename_undo);
        } catch (ENGINE_EXCEPTION error) {
            refresh();
            status.post(error.result == RESULT.not_ready
                ? STRINGS.undo_nothing
                : MESSAGES.result_text(error.result));
            return;
        }
        refresh();
        string text = JOSA_FORMATTER.format(STRINGS.undo_done, report.succeeded);
        if (report.skipped > 0) {
            text += JOSA_FORMATTER.format(STRINGS.undo_done_skipped, report.skipped);
        }
        if (report.failed > 0) {
            text += JOSA_FORMATTER.format(STRINGS.undo_done_failed, report.failed);
        }
        status.post(text);
        if (report.skipped + report.failed > 0) {
            dialogs_.ask(new DIALOG_CONTENT(STRINGS.dialog_undo_result_title,
                JOSA_FORMATTER.format(STRINGS.dialog_undo_result_message, report.succeeded,
                    report.skipped, report.failed),
                report.problems.Select(report_line).ToArray(), "", STRINGS.dialog_ok, ""));
        }
    }

    private async Task resolve(REPORT report)
    {
        status.post(JOSA_FORMATTER.format(STRINGS.rename_choice_prompt, report.total,
            report.failed));
        bool revert = dialogs_.ask(choice_dialog(report));
        RESOLVE_CHOICE choice = revert ? RESOLVE_CHOICE.revert_all : RESOLVE_CHOICE.keep_succeeded;
        REPORT resolved;
        try {
            resolved = await run_engine(STRINGS.progress_renaming,
                progress => engine_.rename_resolve(choice, progress));
        } catch (ENGINE_EXCEPTION error) {
            refresh();
            status.post(MESSAGES.result_text(error.result));
            return;
        }

        refresh();
        REPORT_ITEM[] stuck = resolved.problems.Where(item => item.stage == STAGE.revert)
            .ToArray();
        if (stuck.Length > 0) {
            dialogs_.ask(new DIALOG_CONTENT(STRINGS.dialog_revert_failed_title,
                STRINGS.dialog_revert_failed_message, stuck.Select(report_line).ToArray(), "",
                STRINGS.dialog_ok, ""));
        }
        if (revert) {
            status.post(STRINGS.reverted_all);
            return;
        }
        if (resolved.succeeded == 0) {
            status.post(JOSA_FORMATTER.format(STRINGS.rename_failed_all, resolved.failed));
            return;
        }
        finish_rename(JOSA_FORMATTER.format(STRINGS.renamed_partly, resolved.succeeded,
            resolved.failed));
    }

    private void finish_rename(string text)
    {
        list.clear_selection();
        list.filter = VIEW_FILTER.all;
        status.post(text + STRINGS.renamed_note);
    }

    private async Task<REPORT> run_engine(string label, Func<PROGRESS, REPORT> call)
    {
        PROGRESS progress = new();
        set_running(true);
        status.begin_progress(progress, label);
        try {
            return await Task.Run(() => call(progress));
        } finally {
            status.end_progress();
            set_running(false);
        }
    }

    private void report_blocked()
    {
        List<string> counts = [];
        if (preview_.error > 0) {
            counts.Add(JOSA_FORMATTER.format(STRINGS.rename_count_error, preview_.error));
        }
        if (preview_.conflict > 0) {
            counts.Add(JOSA_FORMATTER.format(STRINGS.rename_count_conflict, preview_.conflict));
        }
        uint first = reveal_first_problem();
        status.post(JOSA_FORMATTER.format(STRINGS.rename_problems,
            string.Join(STRINGS.list_joiner, counts), first));
    }

    private uint reveal_first_problem()
    {
        for (int index = 0; index < preview_.rows.Count; index++) {
            ROW row = preview_.rows[index];
            if (row.status is STATUS.error or STATUS.conflict) {
                list.reveal(row.item_id);
                return (uint)index + 1;
            }
        }
        return 0;
    }

    private DIALOG_CONTENT warning_dialog()
    {
        List<DIALOG_LINE> lines = [];
        for (int index = 0; index < preview_.rows.Count; index++) {
            ROW row = preview_.rows[index];
            if (row.status == STATUS.warning) {
                lines.Add(new DIALOG_LINE(DIALOG_ICON.warning, (uint)index + 1,
                    JOSA_FORMATTER.format(STRINGS.dialog_line, row.name, row.new_name),
                    string.Join(" ", MESSAGES.row_messages(row))));
            }
        }
        return new DIALOG_CONTENT(STRINGS.dialog_warning_title,
            JOSA_FORMATTER.format(STRINGS.dialog_warning_message, preview_.warning,
                preview_.runnable),
            lines, "", STRINGS.dialog_rename, STRINGS.dialog_cancel);
    }

    private DIALOG_CONTENT choice_dialog(REPORT report)
    {
        return new DIALOG_CONTENT(STRINGS.dialog_choice_title,
            JOSA_FORMATTER.format(STRINGS.dialog_choice_message, report.total, report.succeeded,
                report.failed),
            report.problems.Select(report_line).ToArray(), STRINGS.dialog_choice_note,
            STRINGS.dialog_revert_all, STRINGS.dialog_keep);
    }

    private DIALOG_CONTENT failed_dialog(REPORT report)
    {
        return new DIALOG_CONTENT(STRINGS.dialog_failed_title,
            JOSA_FORMATTER.format(STRINGS.dialog_failed_message, report.failed),
            report.problems.Select(report_line).ToArray(), "", STRINGS.dialog_ok, "");
    }

    private DIALOG_CONTENT undo_dialog(REPORT check, uint count)
    {
        string message = JOSA_FORMATTER.format(STRINGS.dialog_undo_message, count);
        if (check.skipped > 0) {
            message += JOSA_FORMATTER.format(STRINGS.dialog_undo_skipped, check.skipped);
        }
        return new DIALOG_CONTENT(STRINGS.dialog_undo_title, message,
            check.problems.Select(report_line).ToArray(), "", STRINGS.dialog_undo_action,
            STRINGS.dialog_cancel);
    }

    private DIALOG_LINE report_line(REPORT_ITEM item)
    {
        string text = JOSA_FORMATTER.format(STRINGS.dialog_line, Path.GetFileName(item.from_path),
            Path.GetFileName(item.to_path));
        string reason = MESSAGES.failure_text(item.failure, item.os_error);
        if (item.stage == STAGE.revert) {
            reason += " " + JOSA_FORMATTER.format(STRINGS.line_current, item.current_path);
        }
        DIALOG_ICON icon = item.stage == STAGE.check ? DIALOG_ICON.warning : DIALOG_ICON.error;
        return new DIALOG_LINE(icon, list.index_of(item.item_id), text, reason);
    }

    private void apply_duplicate(DUPLICATE_OPTIONS options)
    {
        bool custom = options.format == DUPLICATE_FORMAT.custom;
        int length = duplicate_custom_text_.Length;
        if (custom && (length == 0 || length > CUSTOM_TEXT_LIMIT)) {
            return;
        }
        string json = MOD_JSON.write_options(new ENGINE_OPTIONS {
            duplicate = options with { custom_text = custom ? duplicate_custom_text_ : null },
        });
        if (!running_ && attempt(() => engine_.options_set(json))) {
            refresh();
        }
    }

    private bool attempt(Action action)
    {
        try {
            action();
            return true;
        } catch (ENGINE_EXCEPTION error) {
            status.post(MESSAGES.result_text(error.result));
            return false;
        }
    }

    private void set_running(bool running)
    {
        running_ = running;
        notify(nameof(is_running));
        notify(nameof(can_edit));
        refresh_commands();
    }

    private void refresh_commands()
    {
        foreach (COMMAND command in commands_) {
            command.refresh();
        }
    }

    private void on_mods_changed(object? sender, PropertyChangedEventArgs args)
    {
        if (args.PropertyName == nameof(MOD_LIST_VIEW_MODEL.is_editing)) {
            notify(nameof(is_editing));
        }
    }

    private void on_list_changed(object? sender, PropertyChangedEventArgs args)
    {
        if (args.PropertyName == nameof(ITEM_LIST_VIEW_MODEL.selected_ids)) {
            remove_command.refresh();
        }
    }

    private static string theme_name(THEME_MODE mode)
    {
        return mode switch {
            THEME_MODE.light => STRINGS.theme_light,
            THEME_MODE.dark => STRINGS.theme_dark,
            _ => STRINGS.theme_system,
        };
    }

    private static async void fire(Func<Task> action)
    {
        await action();
    }
}
