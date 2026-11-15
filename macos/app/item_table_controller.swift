import AppKit

import limenamer_view_model

@MainActor
final class ITEM_TABLE_CONTROLLER: NSObject, NSTableViewDataSource, NSTableViewDelegate,
    NSMenuDelegate, NSTextFieldDelegate
{
    private static let ROW_HEIGHT: CGFloat = 28
    private static let ROW_VIEW_ID: NSUserInterfaceItemIdentifier =
        NSUserInterfaceItemIdentifier("row")
    private static let DRAG_TYPE: NSPasteboard.PasteboardType =
        NSPasteboard.PasteboardType("io.github.limeheron.limenamer.item")
    private static let DROP_LINE_LEFT: CGFloat = 4
    private static let DROP_LINE_RIGHT: CGFloat = 10
    private static let EDIT_COMMIT_KEYS: [Selector] = [
        #selector(NSResponder.insertNewline(_:)),
        #selector(NSResponder.insertTab(_:)),
        #selector(NSResponder.insertBacktab(_:)),
    ]

    let scroll_view: DROP_SCROLL_VIEW = DROP_SCROLL_VIEW()
    let table: ITEM_TABLE_VIEW = ITEM_TABLE_VIEW()
    private let main: MAIN_VIEW_MODEL
    private lazy var indicator: DROP_INDICATOR = DROP_INDICATOR(table: table,
        scroll_view: scroll_view, left: ITEM_TABLE_CONTROLLER.DROP_LINE_LEFT,
        right: ITEM_TABLE_CONTROLLER.DROP_LINE_RIGHT)
    private let autoscroller: DRAG_AUTOSCROLLER = DRAG_AUTOSCROLLER()
    private var rows: [ITEM_ROW] = []
    private var checked: Set<UInt64> = []
    private var designating: Bool = false
    private var syncing: Bool = false
    private var drag_ids: [UInt64]? = nil
    private var drop_point: NSPoint? = nil
    private var drop_gap: Int = -1
    private var drop_valid: Bool = false
    private var edit_cell: ITEM_CELL? = nil
    private var edit_id: UInt64 = 0
    private var edit_cancelled: Bool = false
    private var edit_suspended: Bool = false

    init(main: MAIN_VIEW_MODEL)
    {
        self.main = main
        super.init()
        setup_table()
        main.list.selection_requested = { [weak self] ids in
            self?.apply_selection(ids)
        }
        main.list.reveal_requested = { [weak self] item_id in
            self?.reveal(item_id)
        }
    }

    func update(rows current: [ITEM_ROW], hidden: Set<ITEM_COLUMN>,
        checked current_checked: Set<UInt64>)
    {
        sync_rows(current)
        for column in table.tableColumns {
            guard let kind = ITEM_COLUMN(rawValue: column.identifier.rawValue) else {
                continue
            }
            let is_hidden: Bool = hidden.contains(kind)
            if column.isHidden != is_hidden {
                column.isHidden = is_hidden
                fit_columns()
            }
        }
        let current_designating: Bool = !hidden.contains(.check)
        if current_checked != checked || current_designating != designating {
            checked = current_checked
            designating = current_designating
            refresh_checks()
        }
    }

    func numberOfRows(in table_view: NSTableView) -> Int
    {
        return rows.count
    }

    func tableView(_ table_view: NSTableView, viewFor table_column: NSTableColumn?, row: Int)
        -> NSView?
    {
        guard let table_column,
            let column = ITEM_COLUMN(rawValue: table_column.identifier.rawValue) else {
            return nil
        }
        let cell: ITEM_CELL = table_view.makeView(withIdentifier: column.identifier, owner: nil)
            as? ITEM_CELL ?? make_cell(column)
        cell.show(rows[row])
        return cell
    }

    func tableView(_ table_view: NSTableView, rowViewForRow row: Int) -> NSTableRowView?
    {
        if let view = table_view.makeView(withIdentifier: ITEM_TABLE_CONTROLLER.ROW_VIEW_ID,
            owner: nil) as? ITEM_ROW_VIEW {
            return view
        }
        let view: ITEM_ROW_VIEW = ITEM_ROW_VIEW()
        view.identifier = ITEM_TABLE_CONTROLLER.ROW_VIEW_ID
        return view
    }

    func tableViewSelectionDidChange(_ notification: Notification)
    {
        if syncing {
            return
        }
        let ids: [UInt64] = table.selectedRowIndexes.compactMap { index in
            return index < rows.count ? rows[index].item_id : nil
        }
        main.list.set_selection(ids)
    }

    func tableView(_ table_view: NSTableView, didClick table_column: NSTableColumn)
    {
        if table_column.identifier == ITEM_COLUMN.check.identifier {
            main.list.check_all(main.list.check_all_state != true)
            refresh_checks()
        }
    }

    func tableView(_ table_view: NSTableView, pasteboardWriterForRow row: Int)
        -> (any NSPasteboardWriting)?
    {
        if !main.can_edit || !main.list.can_drag || edit_cell != nil || row >= rows.count {
            return nil
        }
        let item: NSPasteboardItem = NSPasteboardItem()
        item.setString(String(rows[row].item_id), forType: ITEM_TABLE_CONTROLLER.DRAG_TYPE)
        return item
    }

    func tableView(_ table_view: NSTableView, draggingSession session: NSDraggingSession,
        willBeginAt screen_point: NSPoint, forRowIndexes row_indexes: IndexSet)
    {
        guard let first = row_indexes.first, first < rows.count else {
            return
        }
        drag_ids = main.list.drag_ids(rows[first].item_id)
        drop_gap = -1
        drop_valid = false
        autoscroller.start()
    }

    func tableView(_ table_view: NSTableView, draggingSession session: NSDraggingSession,
        endedAt screen_point: NSPoint, operation: NSDragOperation)
    {
        drag_ids = nil
        autoscroller.stop()
        drop_exited()
    }

    func control(_ control: NSControl, textView: NSTextView, doCommandBy selector: Selector)
        -> Bool
    {
        guard control === edit_cell?.editor else {
            return false
        }
        if selector == #selector(NSResponder.cancelOperation(_:)) {
            edit_cancelled = true
        } else if !ITEM_TABLE_CONTROLLER.EDIT_COMMIT_KEYS.contains(selector) {
            return false
        }
        table.window?.makeFirstResponder(table)
        return true
    }

    func controlTextDidEndEditing(_ notification: Notification)
    {
        guard let cell = edit_cell, let field = cell.editor,
            notification.object as AnyObject? === field else {
            return
        }
        let item_id: UInt64 = edit_id
        let text: String = field.stringValue
        let commit: Bool = !edit_cancelled && !edit_suspended
        cell.end_edit()
        edit_cell = nil
        edit_id = 0
        edit_cancelled = false
        guard commit, let row = rows.first(where: { $0.item_id == item_id }),
            text != row.new_name else {
            return
        }
        Task { @MainActor [main] in
            main.direct_edit(item_id, new_name: text)
        }
    }

    func menuNeedsUpdate(_ menu: NSMenu)
    {
        menu.removeAllItems()
        for column in ITEM_COLUMN.HIDEABLE {
            let item: NSMenuItem = NSMenuItem(title: column.title,
                action: #selector(toggle_column(_:)), keyEquivalent: "")
            item.target = self
            item.representedObject = column.rawValue
            item.state = column.shown(main.settings) ? .on : .off
            menu.addItem(item)
        }
    }

    @objc private func toggle_column(_ sender: NSMenuItem)
    {
        guard let raw = sender.representedObject as? String,
            let column = ITEM_COLUMN(rawValue: raw) else {
            return
        }
        column.set_shown(main.settings, !column.shown(main.settings))
    }

    @objc private func check_clicked(_ sender: NSButton)
    {
        let row: Int = table.row(for: sender)
        guard row >= 0 && row < rows.count else {
            return
        }
        let range: Bool = NSApp.currentEvent?.modifierFlags.contains(.shift) ?? false
        main.list.toggle_check(rows[row].item_id, range: range)
        refresh_checks()
    }

    @objc private func row_clicked(_ sender: Any?)
    {
        let row: Int = table.clickedRow
        if row >= 0 && row < rows.count {
            main.show_row_problems(rows[row].item_id)
        }
    }

    @objc private func row_double_clicked(_ sender: Any?)
    {
        let column: Int = table.clickedColumn
        if column >= 0
            && table.tableColumns[column].identifier == ITEM_COLUMN.new_name.identifier {
            begin_edit(table.clickedRow)
        }
    }

    @objc private func clip_resized(_ notification: Notification)
    {
        fit_columns()
    }

    private func make_cell(_ column: ITEM_COLUMN) -> ITEM_CELL
    {
        let cell: ITEM_CELL = ITEM_CELL(column: column)
        if column == .check {
            cell.bind_check(target: self, action: #selector(check_clicked(_:)))
        }
        return cell
    }

    private func refresh_checks()
    {
        table.enumerateAvailableRowViews { row_view, row in
            guard row >= 0 && row < rows.count else {
                return
            }
            for column in 0..<row_view.numberOfColumns {
                (row_view.view(atColumn: column) as? ITEM_CELL)?.show_check(rows[row])
            }
        }
        let index: Int = table.column(withIdentifier: ITEM_COLUMN.check.identifier)
        guard index >= 0,
            let header = table.tableColumns[index].headerCell as? CHECK_HEADER_CELL else {
            return
        }
        switch main.list.check_all_state {
        case .some(true):
            header.check_state = .on
        case .some(false):
            header.check_state = .off
        case .none:
            header.check_state = .mixed
        }
        table.headerView?.needsDisplay = true
    }

    private func begin_edit(_ row: Int, resume: (text: String, range: NSRange)? = nil)
    {
        let column: Int = table.column(withIdentifier: ITEM_COLUMN.new_name.identifier)
        guard edit_cell == nil, row >= 0, row < rows.count, column >= 0,
            let window = table.window else {
            return
        }
        let item: ITEM_ROW = rows[row]
        if resume == nil {
            if !main.can_start_direct_edit() {
                return
            }
            table.selectRowIndexes(IndexSet(integer: row), byExtendingSelection: false)
        }
        table.scrollRowToVisible(row)
        table.scrollColumnToVisible(column)
        guard let cell = table.view(atColumn: column, row: row, makeIfNecessary: true)
            as? ITEM_CELL else {
            return
        }
        let text: String = resume?.text ?? item.new_name
        let field: NSTextField = cell.begin_edit(text, delegate: self)
        edit_cell = cell
        edit_id = item.item_id
        edit_cancelled = false
        window.makeFirstResponder(field)
        field.currentEditor()?.selectedRange = resume?.range
            ?? ITEM_TABLE_CONTROLLER.name_range(text, is_dir: item.is_dir)
    }

    private func suspend_edit() -> (item_id: UInt64, text: String, range: NSRange)?
    {
        guard let cell = edit_cell, let field = cell.editor else {
            return nil
        }
        let text: String = field.stringValue
        let range: NSRange = field.currentEditor()?.selectedRange
            ?? NSRange(location: 0, length: (text as NSString).length)
        let item_id: UInt64 = edit_id
        edit_suspended = true
        table.window?.makeFirstResponder(table)
        edit_suspended = false
        if edit_cell != nil {
            cell.end_edit()
            edit_cell = nil
            edit_id = 0
        }
        return (item_id, text, range)
    }

    private func fit_columns()
    {
        let width: CGFloat = scroll_view.contentView.bounds.width
        let visible: [NSTableColumn] = table.tableColumns.filter { !$0.isHidden }
        let flexible: [NSTableColumn] = visible.filter { column in
            return ITEM_COLUMN(rawValue: column.identifier.rawValue)?.flexible ?? false
        }
        if width <= 0 || flexible.isEmpty {
            return
        }
        let fixed: CGFloat = visible.reduce(0) { sum, column in
            return flexible.contains(column) ? sum : sum + column.width
        }
        let current: CGFloat = flexible.reduce(0) { $0 + $1.width }
        let target: CGFloat = width - fixed
        if abs(target - current) < 1 {
            return
        }
        table.columnAutoresizingStyle = .noColumnAutoresizing
        var left: CGFloat = target
        for (index, column) in flexible.enumerated() {
            let share: CGFloat = index == flexible.count - 1 ? left
                : floor(target * column.width / max(current, 1))
            column.width = max(column.minWidth, share)
            left -= column.width
        }
        table.columnAutoresizingStyle = .uniformColumnAutoresizingStyle
    }

    private func setup_table()
    {
        table.style = .plain
        table.rowHeight = ITEM_TABLE_CONTROLLER.ROW_HEIGHT
        table.intercellSpacing = NSSize(width: 0, height: 0)
        table.gridStyleMask = []
        table.usesAlternatingRowBackgroundColors = false
        table.allowsMultipleSelection = true
        table.allowsEmptySelection = true
        table.allowsColumnReordering = false
        table.allowsColumnSelection = false
        table.columnAutoresizingStyle = .uniformColumnAutoresizingStyle
        for kind in ITEM_COLUMN.allCases {
            table.addTableColumn(ITEM_TABLE_CONTROLLER.make_column(kind))
        }
        let menu: NSMenu = NSMenu()
        menu.delegate = self
        table.headerView?.menu = menu
        table.dataSource = self
        table.delegate = self
        table.target = self
        table.action = #selector(row_clicked(_:))
        table.doubleAction = #selector(row_double_clicked(_:))
        table.return_pressed = { [weak self] in
            if let self {
                begin_edit(table.selectedRow)
            }
        }
        table.setDraggingSourceOperationMask(.move, forLocal: true)
        table.setDraggingSourceOperationMask([], forLocal: false)
        setup_drop()
        scroll_view.documentView = table
        scroll_view.hasVerticalScroller = true
        scroll_view.hasHorizontalScroller = true
        scroll_view.autohidesScrollers = true
        scroll_view.drawsBackground = true
        scroll_view.contentView.postsFrameChangedNotifications = true
        NotificationCenter.default.addObserver(self, selector: #selector(clip_resized(_:)),
            name: NSView.frameDidChangeNotification, object: scroll_view.contentView)
    }

    private func setup_drop()
    {
        scroll_view.registerForDraggedTypes([ITEM_TABLE_CONTROLLER.DRAG_TYPE])
        scroll_view.drag_updated = { [weak self] info in
            return self?.drop_updated(info) ?? []
        }
        scroll_view.drag_exited = { [weak self] in
            self?.drop_exited()
        }
        scroll_view.drag_performed = { [weak self] info in
            return self?.drop_performed(info) ?? false
        }
        autoscroller.scroll_view = scroll_view
        autoscroller.scrolled = { [weak self] in
            _ = self?.refresh_drop()
        }
    }

    private func drop_updated(_ info: any NSDraggingInfo) -> NSDragOperation
    {
        guard drag_ids != nil, info.draggingSource as AnyObject? === table else {
            drop_exited()
            return []
        }
        drop_point = info.draggingLocation
        return refresh_drop() ? .move : []
    }

    private func drop_performed(_ info: any NSDraggingInfo) -> Bool
    {
        guard let ids = drag_ids, info.draggingSource as AnyObject? === table else {
            drop_exited()
            return false
        }
        autoscroller.stop()
        drop_point = info.draggingLocation
        let valid: Bool = refresh_drop()
        let gap: Int = drop_gap
        drop_exited()
        guard valid, let slot = slot(gap) else {
            return false
        }
        drag_ids = nil
        main.move_rows(ids, target_id: slot.target_id, after: slot.after)
        sync_rows(main.list.rows)
        return true
    }

    private func drop_exited()
    {
        drop_point = nil
        drop_gap = -1
        indicator.hide()
    }

    private func refresh_drop() -> Bool
    {
        guard let ids = drag_ids, let point = drop_point else {
            return false
        }
        let gap: Int = indicator.gap(at: point)
        if gap != drop_gap {
            drop_gap = gap
            drop_valid = slot(gap).map { slot in
                return main.list.can_drop(ids, target_id: slot.target_id, after: slot.after)
            } ?? false
        }
        if drop_valid {
            indicator.show(gap)
        } else {
            indicator.hide()
        }
        return drop_valid
    }

    private func sync_rows(_ current: [ITEM_ROW])
    {
        if current.count == rows.count && current.elementsEqual(rows, by: ===) {
            return
        }
        let resume: (item_id: UInt64, text: String, range: NSRange)? = suspend_edit()
        rows = current
        syncing = true
        table.reloadData()
        syncing = false
        apply_selection(main.list.selected_ids)
        if let resume, let row = rows.firstIndex(where: { $0.item_id == resume.item_id }) {
            begin_edit(row, resume: (resume.text, resume.range))
        }
    }

    private func apply_selection(_ ids: Set<UInt64>)
    {
        sync_rows(main.list.rows)
        var indexes: IndexSet = IndexSet()
        if !ids.isEmpty {
            for (index, row) in rows.enumerated() where ids.contains(row.item_id) {
                indexes.insert(index)
            }
        }
        if indexes == table.selectedRowIndexes {
            return
        }
        syncing = true
        table.selectRowIndexes(indexes, byExtendingSelection: false)
        syncing = false
    }

    private func reveal(_ item_id: UInt64)
    {
        sync_rows(main.list.rows)
        if let index = rows.firstIndex(where: { $0.item_id == item_id }) {
            table.scrollRowToVisible(index)
        }
    }

    private func slot(_ gap: Int) -> (target_id: UInt64, after: Bool)?
    {
        if rows.isEmpty || gap < 0 || gap > rows.count {
            return nil
        }
        if gap == 0 {
            return (rows[0].item_id, false)
        }
        return (rows[gap - 1].item_id, true)
    }

    private static func name_range(_ text: String, is_dir: Bool) -> NSRange
    {
        let name: NSString = text as NSString
        let dot: Int = is_dir ? NSNotFound : name.range(of: ".", options: .backwards).location
        if dot != NSNotFound && dot > 0 {
            return NSRange(location: 0, length: dot)
        }
        return NSRange(location: 0, length: name.length)
    }

    private static func make_column(_ kind: ITEM_COLUMN) -> NSTableColumn
    {
        let column: NSTableColumn = NSTableColumn(identifier: kind.identifier)
        column.title = kind.title
        column.width = kind.width
        column.minWidth = kind.min_width
        let font: NSFont = column.headerCell.font ?? NSFont.systemFont(ofSize: 11)
        if kind == .check {
            column.maxWidth = kind.width
            column.resizingMask = []
            column.headerToolTip = STRINGS.check_all_tip
            column.headerCell = CHECK_HEADER_CELL(textCell: "")
            return column
        }
        column.resizingMask = kind.flexible ? [.autoresizingMask, .userResizingMask]
            : .userResizingMask
        let header: ITEM_HEADER_CELL = ITEM_HEADER_CELL(textCell: kind.title)
        header.font = font
        header.alignment = kind.alignment
        if kind == .new_name {
            header.attributedStringValue = NSAttributedString(string: kind.title,
                attributes: [.foregroundColor: PALETTE.ACCENT_TEXT,
                    .font: NSFont.systemFont(ofSize: font.pointSize, weight: .bold)])
        }
        column.headerCell = header
        return column
    }
}
