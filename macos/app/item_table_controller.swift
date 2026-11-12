import AppKit

import limenamer_view_model

@MainActor
final class ITEM_TABLE_CONTROLLER: NSObject, NSTableViewDataSource, NSTableViewDelegate,
    NSMenuDelegate
{
    private static let ROW_HEIGHT: CGFloat = 28
    private static let ROW_VIEW_ID: NSUserInterfaceItemIdentifier =
        NSUserInterfaceItemIdentifier("row")

    let scroll_view: NSScrollView = NSScrollView()
    let table: NSTableView = NSTableView()
    private let main: MAIN_VIEW_MODEL
    private var rows: [ITEM_ROW] = []
    private var syncing: Bool = false

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

    func update(rows current: [ITEM_ROW], hidden: Set<ITEM_COLUMN>)
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
            as? ITEM_CELL ?? ITEM_CELL(column: column)
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

    @objc private func clip_resized(_ notification: Notification)
    {
        fit_columns()
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
        scroll_view.documentView = table
        scroll_view.hasVerticalScroller = true
        scroll_view.hasHorizontalScroller = true
        scroll_view.autohidesScrollers = true
        scroll_view.drawsBackground = true
        scroll_view.contentView.postsFrameChangedNotifications = true
        NotificationCenter.default.addObserver(self, selector: #selector(clip_resized(_:)),
            name: NSView.frameDidChangeNotification, object: scroll_view.contentView)
    }

    private func sync_rows(_ current: [ITEM_ROW])
    {
        if current.count == rows.count && current.elementsEqual(rows, by: ===) {
            return
        }
        rows = current
        syncing = true
        table.reloadData()
        syncing = false
        apply_selection(main.list.selected_ids)
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

    private static func make_column(_ kind: ITEM_COLUMN) -> NSTableColumn
    {
        let column: NSTableColumn = NSTableColumn(identifier: kind.identifier)
        column.title = kind.title
        column.width = kind.width
        column.minWidth = kind.min_width
        column.resizingMask = kind.flexible ? [.autoresizingMask, .userResizingMask]
            : .userResizingMask
        let header: ITEM_HEADER_CELL = ITEM_HEADER_CELL(textCell: kind.title)
        let font: NSFont = column.headerCell.font ?? NSFont.systemFont(ofSize: 11)
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
