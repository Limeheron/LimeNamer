import AppKit

import limenamer_view_model

@MainActor
final class MOD_CARD_CONTROLLER: NSObject, NSTableViewDataSource, NSTableViewDelegate
{
    private static let DRAG_TYPE: NSPasteboard.PasteboardType =
        NSPasteboard.PasteboardType("io.github.limeheron.limenamer.mod")
    private static let COLUMN_ID: NSUserInterfaceItemIdentifier =
        NSUserInterfaceItemIdentifier("cards")
    private static let BOTTOM_INSET: CGFloat = 9
    private static let DROP_LINE_LEFT: CGFloat = 6
    private static let DROP_LINE_RIGHT: CGFloat = 12

    let scroll_view: DROP_SCROLL_VIEW = DROP_SCROLL_VIEW()
    let table: NSTableView = NSTableView()
    private let main: MAIN_VIEW_MODEL
    private let autoscroller: DRAG_AUTOSCROLLER = DRAG_AUTOSCROLLER()
    private lazy var indicator: DROP_INDICATOR = DROP_INDICATOR(table: table,
        scroll_view: scroll_view, left: MOD_CARD_CONTROLLER.DROP_LINE_LEFT,
        right: MOD_CARD_CONTROLLER.DROP_LINE_RIGHT)
    private var cards: [MOD_CARD] = []
    private var enabled: Bool = true
    private var width: CGFloat = 0
    private var drag_id: UInt64? = nil
    private var drop_point: NSPoint? = nil
    private var drop_gap: Int = -1
    private var drop_valid: Bool = false

    init(main: MAIN_VIEW_MODEL)
    {
        self.main = main
        super.init()
        setup_table()
        setup_drop()
    }

    func update(_ current: [MOD_CARD], enabled current_enabled: Bool)
    {
        if current != cards || current_enabled != enabled {
            cards = current
            enabled = current_enabled
            table.reloadData()
        }
    }

    func numberOfRows(in table_view: NSTableView) -> Int
    {
        return cards.count
    }

    func tableView(_ table_view: NSTableView, viewFor table_column: NSTableColumn?, row: Int)
        -> NSView?
    {
        let cell: MOD_CARD_VIEW = table_view.makeView(withIdentifier: MOD_CARD_VIEW.IDENTIFIER,
            owner: nil) as? MOD_CARD_VIEW ?? MOD_CARD_VIEW()
        let card: MOD_CARD = cards[row]
        cell.show(card, enabled: enabled)
        cell.toggled = { [weak self] in
            self?.toggle(card)
        }
        cell.removed = { [weak self] in
            self?.remove(card)
        }
        return cell
    }

    func tableView(_ table_view: NSTableView, heightOfRow row: Int) -> CGFloat
    {
        return MOD_CARD_VIEW.height(cards[row].summary, width: card_width())
    }

    func tableView(_ table_view: NSTableView, shouldSelectRow row: Int) -> Bool
    {
        return false
    }

    func tableView(_ table_view: NSTableView, pasteboardWriterForRow row: Int)
        -> (any NSPasteboardWriting)?
    {
        if !enabled || !main.can_edit || row >= cards.count {
            return nil
        }
        let item: NSPasteboardItem = NSPasteboardItem()
        item.setString(String(cards[row].id), forType: MOD_CARD_CONTROLLER.DRAG_TYPE)
        return item
    }

    func tableView(_ table_view: NSTableView, draggingSession session: NSDraggingSession,
        willBeginAt screen_point: NSPoint, forRowIndexes row_indexes: IndexSet)
    {
        guard let first = row_indexes.first, first < cards.count else {
            return
        }
        drag_id = cards[first].id
        drop_gap = -1
        drop_valid = false
        autoscroller.start()
    }

    func tableView(_ table_view: NSTableView, draggingSession session: NSDraggingSession,
        endedAt screen_point: NSPoint, operation: NSDragOperation)
    {
        drag_id = nil
        autoscroller.stop()
        drop_exited()
    }

    @objc private func card_clicked(_ sender: Any?)
    {
        let row: Int = table.clickedRow
        if enabled && row >= 0 && row < cards.count && !clicked_button() {
            main.mods.open_card(cards[row])
        }
    }

    @objc private func clip_resized(_ notification: Notification)
    {
        let current: CGFloat = card_width()
        if abs(current - width) > 0.5 {
            width = current
            table.noteHeightOfRows(withIndexesChanged: IndexSet(integersIn: 0..<cards.count))
        }
    }

    private func toggle(_ card: MOD_CARD)
    {
        MOD_ACTIONS.run(main) { () throws(ENGINE_ERROR) in
            try main.mods.set_enabled(card, enabled: !card.enabled)
        }
    }

    private func remove(_ card: MOD_CARD)
    {
        MOD_ACTIONS.run(main) { () throws(ENGINE_ERROR) in
            try main.mods.remove(card)
        }
    }

    private func clicked_button() -> Bool
    {
        guard let event = NSApp.currentEvent, let parent = table.superview else {
            return false
        }
        var view: NSView? = table.hitTest(parent.convert(event.locationInWindow, from: nil))
        while let current = view, current !== table {
            if current is NSButton {
                return true
            }
            view = current.superview
        }
        return false
    }

    private func card_width() -> CGFloat
    {
        return scroll_view.contentView.bounds.width
    }

    private func setup_table()
    {
        let column: NSTableColumn = NSTableColumn(identifier: MOD_CARD_CONTROLLER.COLUMN_ID)
        column.resizingMask = .autoresizingMask
        table.addTableColumn(column)
        table.style = .plain
        table.headerView = nil
        table.intercellSpacing = NSSize(width: 0, height: 0)
        table.gridStyleMask = []
        table.backgroundColor = .clear
        table.selectionHighlightStyle = .none
        table.focusRingType = .none
        table.columnAutoresizingStyle = .firstColumnOnlyAutoresizingStyle
        table.dataSource = self
        table.delegate = self
        table.target = self
        table.action = #selector(card_clicked(_:))
        table.setDraggingSourceOperationMask(.move, forLocal: true)
        table.setDraggingSourceOperationMask([], forLocal: false)
        scroll_view.documentView = table
        scroll_view.drawsBackground = false
        scroll_view.hasVerticalScroller = true
        scroll_view.autohidesScrollers = true
        scroll_view.automaticallyAdjustsContentInsets = false
        scroll_view.contentInsets = NSEdgeInsets(top: 0, left: 0,
            bottom: MOD_CARD_CONTROLLER.BOTTOM_INSET, right: 0)
        scroll_view.contentView.postsFrameChangedNotifications = true
        NotificationCenter.default.addObserver(self, selector: #selector(clip_resized(_:)),
            name: NSView.frameDidChangeNotification, object: scroll_view.contentView)
    }

    private func setup_drop()
    {
        scroll_view.registerForDraggedTypes([MOD_CARD_CONTROLLER.DRAG_TYPE])
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
        guard drag_id != nil, info.draggingSource as AnyObject? === table else {
            drop_exited()
            return []
        }
        drop_point = info.draggingLocation
        return refresh_drop() ? .move : []
    }

    private func drop_performed(_ info: any NSDraggingInfo) -> Bool
    {
        guard let id = drag_id, info.draggingSource as AnyObject? === table else {
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
        drag_id = nil
        var moved: Bool = false
        MOD_ACTIONS.run(main) { () throws(ENGINE_ERROR) in
            moved = try main.mods.drop(id, target_id: slot.target_id, after: slot.after)
        }
        return moved
    }

    private func drop_exited()
    {
        drop_point = nil
        drop_gap = -1
        indicator.hide()
    }

    private func refresh_drop() -> Bool
    {
        guard let id = drag_id, let point = drop_point else {
            return false
        }
        let gap: Int = indicator.gap(at: point)
        if gap != drop_gap {
            drop_gap = gap
            drop_valid = slot(gap).map { slot in
                return main.mods.can_drop(id, target_id: slot.target_id, after: slot.after)
            } ?? false
        }
        if drop_valid {
            indicator.show(gap)
        } else {
            indicator.hide()
        }
        return drop_valid
    }

    private func slot(_ gap: Int) -> (target_id: UInt64, after: Bool)?
    {
        if cards.isEmpty || gap < 0 || gap > cards.count {
            return nil
        }
        if gap == 0 {
            return (cards[0].id, false)
        }
        return (cards[gap - 1].id, true)
    }
}
