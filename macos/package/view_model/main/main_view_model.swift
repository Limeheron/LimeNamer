import Foundation
import Observation

@MainActor
@Observable
public final class MAIN_VIEW_MODEL
{
    public static let SORT_CHOICES: [CHOICE<SORT_KEY>] = [
        CHOICE(value: .added, label: STRINGS.sort_added),
        CHOICE(value: .name_natural, label: STRINGS.sort_name_natural),
        CHOICE(value: .name_ordinal, label: STRINGS.sort_name_ordinal),
        CHOICE(value: .modified, label: STRINGS.sort_modified),
    ]
    public static let DUPLICATE_FORMAT_CHOICES: [CHOICE<DUPLICATE_FORMAT>] = [
        CHOICE(value: .paren, label: STRINGS.duplicate_paren),
        CHOICE(value: .underscore, label: STRINGS.duplicate_underscore),
        CHOICE(value: .dash, label: STRINGS.duplicate_dash),
        CHOICE(value: .custom, label: STRINGS.duplicate_custom),
    ]
    public static let DUPLICATE_MODE_CHOICES: [CHOICE<DUPLICATE_MODE>] = [
        CHOICE(value: .keep_first, label: STRINGS.duplicate_keep_first),
        CHOICE(value: .number_all, label: STRINGS.duplicate_number_all),
    ]
    public static let THEME_CHOICES: [CHOICE<THEME_MODE>] = [
        CHOICE(value: .system, label: STRINGS.theme_system),
        CHOICE(value: .light, label: STRINGS.theme_light),
        CHOICE(value: .dark, label: STRINGS.theme_dark),
    ]
    private static let CUSTOM_TEXT_LIMIT: Int = 32

    public let list: ITEM_LIST_VIEW_MODEL
    public let mods: MOD_LIST_VIEW_MODEL
    public let status: STATUS_BAR_VIEW_MODEL
    public let settings: SETTINGS
    public private(set) var is_running: Bool = false
    public private(set) var sort_key: SORT_KEY = .added
    public private(set) var sort_descending: Bool = false
    @ObservationIgnored private let engine: ENGINE
    @ObservationIgnored private let dialogs: any DIALOG_SERVICE
    private var preview: PREVIEW = PREVIEW(rows: [], changed: 0, runnable: 0, warning: 0,
        error: 0, conflict: 0)
    private var history_undo: Bool = false
    private var history_redo: Bool = false
    private var rename_undo_ready: Bool = false
    private var duplicate: DUPLICATE_OPTIONS = DUPLICATE_OPTIONS()
    private var custom_text: String = "_copy"
    @ObservationIgnored private var choosing: Bool = false

    public init(engine: ENGINE, settings: SETTINGS, dialogs: any DIALOG_SERVICE,
        now: @escaping () -> Date)
    {
        self.engine = engine
        self.settings = settings
        self.dialogs = dialogs
        list = ITEM_LIST_VIEW_MODEL(engine: engine)
        mods = MOD_LIST_VIEW_MODEL(engine: engine, list: list)
        status = STATUS_BAR_VIEW_MODEL(list: list, now: now)
        mods.changed = { [weak self] text in
            self?.refresh()
            self?.status.post(text)
        }
        mods.notice = { [weak self] text in
            self?.status.post(text)
        }
        refresh()
    }

    public var can_edit: Bool
    {
        return !is_running
    }

    public var is_editing: Bool
    {
        return mods.is_editing
    }

    public var can_remove: Bool
    {
        return !is_running && !list.selected_ids.isEmpty
    }

    public var can_clear: Bool
    {
        return !is_running && list.count_all > 0
    }

    public var can_undo: Bool
    {
        return !is_running && history_undo
    }

    public var can_redo: Bool
    {
        return !is_running && history_redo
    }

    public var can_rename: Bool
    {
        return !is_running && preview.changed > 0
    }

    public var can_rename_undo: Bool
    {
        return !is_running && rename_undo_ready
    }

    public var can_toggle_panel: Bool
    {
        return !is_running
    }

    public var panel_open: Bool
    {
        get {
            return settings.panel_open
        }
        set {
            if !newValue && mods.is_editing {
                status.post(JOSA_FORMATTER.format(STRINGS.blocked_panel, STRINGS.edit_block))
                return
            }
            settings.panel_open = newValue
        }
    }

    public var shows_panel_badge: Bool
    {
        return !settings.panel_open && mods.enabled_count > 0
    }

    public var duplicate_enabled: Bool
    {
        get {
            return duplicate.enabled
        }
        set {
            var options: DUPLICATE_OPTIONS = duplicate
            options.enabled = newValue
            apply_duplicate(options)
        }
    }

    public var duplicate_format: DUPLICATE_FORMAT
    {
        get {
            return duplicate.format
        }
        set {
            var options: DUPLICATE_OPTIONS = duplicate
            options.format = newValue
            apply_duplicate(options)
        }
    }

    public var duplicate_is_custom: Bool
    {
        return duplicate.format == .custom
    }

    public var duplicate_mode: DUPLICATE_MODE
    {
        get {
            return duplicate.mode
        }
        set {
            var options: DUPLICATE_OPTIONS = duplicate
            options.mode = newValue
            apply_duplicate(options)
        }
    }

    public var duplicate_custom_text: String
    {
        get {
            return custom_text
        }
        set {
            custom_text = newValue
            apply_duplicate(duplicate)
        }
    }

    public func refresh()
    {
        do throws(ENGINE_ERROR) {
            preview = try engine.preview()
            list.load(preview)
            try mods.load(preview)
            (history_undo, history_redo) = try engine.history_state()
            rename_undo_ready = try engine.rename_undo_state().available
            (sort_key, sort_descending) = try engine.items_sort_state()
            let json: String = try engine.options_get()
            guard let options = try? MOD_JSON.parse_options(json) else {
                throw ENGINE_ERROR(result: .internal_error)
            }
            duplicate = options.duplicate
            if let text = options.duplicate.custom_text {
                custom_text = text
            }
        } catch {
            status.post(MESSAGES.result_text(error.result))
        }
    }

    public func add_paths(_ paths: [String], mode: ADD_MODE)
    {
        if is_running || paths.isEmpty {
            return
        }
        var result = ADD_RESULT(added: 0, skipped: 0, failed: 0)
        if !attempt({ () throws(ENGINE_ERROR) in
            result = try engine.items_add(paths, mode: mode)
        }) {
            return
        }
        refresh()
        var text: String = JOSA_FORMATTER.format(STRINGS.items_added, result.added)
        if result.skipped > 0 {
            text += JOSA_FORMATTER.format(STRINGS.items_added_skipped, result.skipped)
        }
        if result.failed > 0 {
            text += JOSA_FORMATTER.format(STRINGS.items_added_failed, result.failed)
        }
        status.post(text)
    }

    public func remove_selected()
    {
        let ids: [UInt64] = Array(list.selected_ids)
        if is_running || ids.isEmpty {
            return
        }
        if !attempt({ () throws(ENGINE_ERROR) in try engine.items_remove(ids) }) {
            return
        }
        refresh()
        status.post(JOSA_FORMATTER.format(STRINGS.items_removed, ids.count))
    }

    public func clear()
    {
        if is_running || !attempt({ () throws(ENGINE_ERROR) in try engine.items_clear() }) {
            return
        }
        refresh()
        status.post(STRINGS.items_cleared)
    }

    public func undo()
    {
        if is_running || !history_undo {
            return
        }
        if mods.is_editing {
            status.post(JOSA_FORMATTER.format(STRINGS.blocked_undo, STRINGS.edit_block))
            return
        }
        if attempt({ () throws(ENGINE_ERROR) in try engine.undo() }) {
            refresh()
            status.post(STRINGS.undone)
        }
    }

    public func redo()
    {
        if is_running || !history_redo {
            return
        }
        if mods.is_editing {
            status.post(JOSA_FORMATTER.format(STRINGS.blocked_redo, STRINGS.edit_block))
            return
        }
        if attempt({ () throws(ENGINE_ERROR) in try engine.redo() }) {
            refresh()
            status.post(STRINGS.redone)
        }
    }

    public func sort(_ key: SORT_KEY, descending: Bool)
    {
        if !is_running
            && attempt({ () throws(ENGINE_ERROR) in
                try engine.items_sort(key, descending: descending)
            }) {
            refresh()
        }
    }

    public func move_rows(_ ids: [UInt64], target_id: UInt64, after: Bool)
    {
        if is_running {
            return
        }
        var moved: Bool = false
        if !attempt({ () throws(ENGINE_ERROR) in
            moved = try list.drop(ids, target_id: target_id, after: after)
        }) || !moved {
            return
        }
        refresh()
        status.post(ids.count > 1
            ? JOSA_FORMATTER.format(STRINGS.items_moved_many, ids.count)
            : STRINGS.items_moved)
    }

    public func toggle_panel()
    {
        if !is_running {
            panel_open = !panel_open
        }
    }

    public func can_start_direct_edit() -> Bool
    {
        if is_running {
            return false
        }
        if mods.is_editing {
            status.post(STRINGS.blocked_direct)
            return false
        }
        return true
    }

    public func direct_edit(_ item_id: UInt64, new_name: String)
    {
        if is_running {
            return
        }
        if mods.is_editing {
            status.post(STRINGS.blocked_direct)
            return
        }
        if attempt({ () throws(ENGINE_ERROR) in
            try engine.mod_direct_edit(item_id, new_name: new_name)
        }) {
            refresh()
            status.post(JOSA_FORMATTER.format(STRINGS.direct_edited, list.index_of(item_id)))
        }
    }

    public func show_row_problems(_ item_id: UInt64)
    {
        guard let row = list.rows.first(where: { $0.item_id == item_id }) else {
            return
        }
        let problems: [String] = row.problems
        if !problems.isEmpty {
            status.post(JOSA_FORMATTER.format(STRINGS.row_problems, row.index,
                problems.joined(separator: " ")))
        }
    }

    public func location_of(_ item_id: UInt64) -> String?
    {
        guard let row = list.rows.first(where: { $0.item_id == item_id }),
            FileManager.default.fileExists(atPath: row.full_path) else {
            status.post(STRINGS.location_missing)
            return nil
        }
        return row.full_path
    }

    public func activated()
    {
        if !is_running && !choosing
                && attempt({ () throws(ENGINE_ERROR) in try engine.folders_refresh() }) {
            refresh()
        }
    }

    public func allow_close() -> Bool
    {
        if is_running || choosing {
            status.post(STRINGS.blocked_close)
            return false
        }
        return true
    }

    public func set_theme(_ mode: THEME_MODE)
    {
        settings.theme_mode = mode
        status.post(JOSA_FORMATTER.format(STRINGS.theme_changed,
            MAIN_VIEW_MODEL.theme_name(mode)))
    }

    public func toggle_theme(system_dark: Bool)
    {
        let dark: Bool = settings.theme_mode == .dark
            || (settings.theme_mode == .system && system_dark)
        set_theme(dark ? .light : .dark)
    }

    public func rename() async
    {
        if is_running {
            return
        }
        if mods.is_editing {
            status.post(JOSA_FORMATTER.format(STRINGS.blocked_rename, STRINGS.edit_block))
            return
        }
        if preview.error + preview.conflict > 0 {
            report_blocked()
            return
        }
        if preview.changed == 0 {
            status.post(STRINGS.rename_nothing)
            return
        }
        if preview.warning > 0 {
            let confirmed: Bool = await dialogs.ask(warning_dialog())
            if !confirmed {
                return
            }
        }

        let report: REPORT
        do throws(ENGINE_ERROR) {
            report = try await run_engine(STRINGS.progress_renaming) {
                [engine] (progress: PROGRESS) throws(ENGINE_ERROR) -> REPORT in
                return try engine.rename(progress: progress)
            }
        } catch {
            refresh()
            if error.result == .not_ready {
                status.post(STRINGS.rename_not_ready)
                reveal_first_problem()
            } else {
                status.post(MESSAGES.result_text(error.result))
            }
            return
        }

        if report.needs_choice {
            await resolve(report)
            return
        }
        refresh()
        if report.failed > 0 {
            _ = await dialogs.ask(failed_dialog(report))
        }
        if report.succeeded == 0 {
            status.post(JOSA_FORMATTER.format(STRINGS.rename_failed_all, report.failed))
            return
        }
        finish_rename(report.failed > 0
            ? JOSA_FORMATTER.format(STRINGS.renamed_partly, report.succeeded, report.failed)
            : JOSA_FORMATTER.format(STRINGS.renamed, report.succeeded))
    }

    public func rename_undo() async
    {
        if is_running || !rename_undo_ready {
            return
        }
        if mods.is_editing {
            status.post(JOSA_FORMATTER.format(STRINGS.blocked_rename_undo, STRINGS.edit_block))
            return
        }
        var check = REPORT(problems: [], total: 0, succeeded: 0, skipped: 0, failed: 0,
            needs_choice: false)
        if !attempt({ () throws(ENGINE_ERROR) in check = try engine.rename_undo_check() }) {
            return
        }
        let count: UInt32 = check.total - check.skipped
        if count == 0 {
            status.post(STRINGS.undo_nothing)
            return
        }
        let confirmed: Bool = await dialogs.ask(undo_dialog(check, count))
        if !confirmed {
            return
        }

        let report: REPORT
        do throws(ENGINE_ERROR) {
            report = try await run_engine(STRINGS.progress_reverting) {
                [engine] (progress: PROGRESS) throws(ENGINE_ERROR) -> REPORT in
                return try engine.rename_undo(progress: progress)
            }
        } catch {
            refresh()
            status.post(error.result == .not_ready
                ? STRINGS.undo_nothing
                : MESSAGES.result_text(error.result))
            return
        }
        refresh()
        var text: String = JOSA_FORMATTER.format(STRINGS.undo_done, report.succeeded)
        if report.skipped > 0 {
            text += JOSA_FORMATTER.format(STRINGS.undo_done_skipped, report.skipped)
        }
        if report.failed > 0 {
            text += JOSA_FORMATTER.format(STRINGS.undo_done_failed, report.failed)
        }
        status.post(text)
        if report.skipped + report.failed > 0 {
            _ = await dialogs.ask(DIALOG_CONTENT(kind: .report,
                title: STRINGS.dialog_undo_result_title,
                message: JOSA_FORMATTER.format(STRINGS.dialog_undo_result_message,
                    report.succeeded, report.skipped, report.failed),
                lines: report.problems.map { report_line($0) }, note: "",
                primary: STRINGS.dialog_ok, secondary: ""))
        }
    }

    private func resolve(_ report: REPORT) async
    {
        status.post(JOSA_FORMATTER.format(STRINGS.rename_choice_prompt, report.total,
            report.failed))
        choosing = true
        let revert: Bool = await dialogs.ask(choice_dialog(report))
        choosing = false
        let choice: RESOLVE_CHOICE = revert ? .revert_all : .keep_succeeded
        let resolved: REPORT
        do throws(ENGINE_ERROR) {
            resolved = try await run_engine(STRINGS.progress_renaming) {
                [engine] (progress: PROGRESS) throws(ENGINE_ERROR) -> REPORT in
                return try engine.rename_resolve(choice, progress: progress)
            }
        } catch {
            refresh()
            status.post(MESSAGES.result_text(error.result))
            return
        }

        refresh()
        let stuck: [REPORT_ITEM] = resolved.problems.filter { $0.stage == .revert }
        if !stuck.isEmpty {
            _ = await dialogs.ask(DIALOG_CONTENT(kind: .report,
                title: STRINGS.dialog_revert_failed_title,
                message: STRINGS.dialog_revert_failed_message,
                lines: stuck.map { report_line($0) }, note: "", primary: STRINGS.dialog_ok,
                secondary: ""))
        }
        if revert {
            status.post(stuck.isEmpty
                ? STRINGS.reverted_all
                : JOSA_FORMATTER.format(STRINGS.reverted_partly, stuck.count))
            return
        }
        if resolved.succeeded == 0 {
            status.post(JOSA_FORMATTER.format(STRINGS.rename_failed_all, resolved.failed))
            return
        }
        finish_rename(JOSA_FORMATTER.format(STRINGS.renamed_partly, resolved.succeeded,
            resolved.failed))
    }

    private func finish_rename(_ text: String)
    {
        list.clear_selection()
        list.filter = .all
        status.post(text + STRINGS.renamed_note)
    }

    private func run_engine(_ label: String,
        _ call: @escaping @Sendable (PROGRESS) throws(ENGINE_ERROR) -> REPORT)
        async throws(ENGINE_ERROR) -> REPORT
    {
        let progress = PROGRESS()
        is_running = true
        status.begin_progress(progress, label: label)
        defer {
            status.end_progress()
            is_running = false
        }
        let result: Result<REPORT, ENGINE_ERROR> = await Task.detached {
            return Result { () throws(ENGINE_ERROR) -> REPORT in
                return try call(progress)
            }
        }.value
        return try result.get()
    }

    private func report_blocked()
    {
        var counts: [String] = []
        if preview.error > 0 {
            counts.append(JOSA_FORMATTER.format(STRINGS.rename_count_error, preview.error))
        }
        if preview.conflict > 0 {
            counts.append(JOSA_FORMATTER.format(STRINGS.rename_count_conflict,
                preview.conflict))
        }
        let first: UInt32 = reveal_first_problem()
        status.post(JOSA_FORMATTER.format(STRINGS.rename_problems,
            counts.joined(separator: STRINGS.list_joiner), first))
    }

    @discardableResult
    private func reveal_first_problem() -> UInt32
    {
        for (offset, row) in preview.rows.enumerated() {
            if row.status == .error || row.status == .conflict {
                list.reveal(row.item_id)
                return UInt32(offset + 1)
            }
        }
        return 0
    }

    private func warning_dialog() -> DIALOG_CONTENT
    {
        var lines: [DIALOG_LINE] = []
        for (offset, row) in preview.rows.enumerated() where row.status == .warning {
            lines.append(DIALOG_LINE(icon: .warning, index: UInt32(offset + 1),
                text: JOSA_FORMATTER.format(STRINGS.dialog_line, row.name, row.new_name),
                reason: MESSAGES.row_messages(row).joined(separator: " ")))
        }
        return DIALOG_CONTENT(kind: .confirm, title: STRINGS.dialog_warning_title,
            message: JOSA_FORMATTER.format(STRINGS.dialog_warning_message, preview.warning,
                preview.runnable),
            lines: lines, note: "", primary: STRINGS.dialog_rename,
            secondary: STRINGS.dialog_cancel)
    }

    private func choice_dialog(_ report: REPORT) -> DIALOG_CONTENT
    {
        return DIALOG_CONTENT(kind: .choice, title: STRINGS.dialog_choice_title,
            message: JOSA_FORMATTER.format(STRINGS.dialog_choice_message, report.total,
                report.succeeded, report.failed),
            lines: report.problems.map { report_line($0) }, note: STRINGS.dialog_choice_note,
            primary: STRINGS.dialog_revert_all, secondary: STRINGS.dialog_keep)
    }

    private func failed_dialog(_ report: REPORT) -> DIALOG_CONTENT
    {
        return DIALOG_CONTENT(kind: .report, title: STRINGS.dialog_failed_title,
            message: JOSA_FORMATTER.format(STRINGS.dialog_failed_message, report.failed),
            lines: report.problems.map { report_line($0) }, note: "",
            primary: STRINGS.dialog_ok, secondary: "")
    }

    private func undo_dialog(_ check: REPORT, _ count: UInt32) -> DIALOG_CONTENT
    {
        var message: String = JOSA_FORMATTER.format(STRINGS.dialog_undo_message, count)
        if check.skipped > 0 {
            message += JOSA_FORMATTER.format(STRINGS.dialog_undo_skipped, check.skipped)
        }
        return DIALOG_CONTENT(kind: .confirm, title: STRINGS.dialog_undo_title,
            message: message, lines: check.problems.map { report_line($0) }, note: "",
            primary: STRINGS.dialog_undo_action, secondary: STRINGS.dialog_cancel)
    }

    private func report_line(_ item: REPORT_ITEM) -> DIALOG_LINE
    {
        let text: String = JOSA_FORMATTER.format(STRINGS.dialog_line,
            (item.from_path as NSString).lastPathComponent,
            (item.to_path as NSString).lastPathComponent)
        var reason: String = MESSAGES.failure_text(item.failure, os_error: item.os_error)
        if item.stage == .revert {
            reason += " " + JOSA_FORMATTER.format(STRINGS.line_current, item.current_path)
        }
        let icon: DIALOG_ICON = item.stage == .check ? .warning : .error
        return DIALOG_LINE(icon: icon, index: list.index_of(item.item_id), text: text,
            reason: reason)
    }

    private func apply_duplicate(_ options: DUPLICATE_OPTIONS)
    {
        let custom: Bool = options.format == .custom
        let length: Int = custom_text.unicodeScalars.count
        if custom && (length == 0 || length > MAIN_VIEW_MODEL.CUSTOM_TEXT_LIMIT) {
            return
        }
        var next: DUPLICATE_OPTIONS = options
        next.custom_text = custom ? custom_text : nil
        let json: String = MOD_JSON.write_options(ENGINE_OPTIONS(duplicate: next))
        if !is_running && attempt({ () throws(ENGINE_ERROR) in try engine.options_set(json) }) {
            refresh()
        }
    }

    private func attempt(_ action: () throws(ENGINE_ERROR) -> Void) -> Bool
    {
        do {
            try action()
            return true
        } catch {
            status.post(MESSAGES.result_text(error.result))
            return false
        }
    }

    private static func theme_name(_ mode: THEME_MODE) -> String
    {
        switch mode {
        case .light:
            return STRINGS.theme_light
        case .dark:
            return STRINGS.theme_dark
        case .system:
            return STRINGS.theme_system
        }
    }
}
