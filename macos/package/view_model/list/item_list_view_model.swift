import Observation

@MainActor
@Observable
public final class ITEM_LIST_VIEW_MODEL
{
    @ObservationIgnored public var selection_requested: ((Set<UInt64>) -> Void)? = nil
    @ObservationIgnored public var reveal_requested: ((UInt64) -> Void)? = nil
    public private(set) var rows: [ITEM_ROW] = []
    public private(set) var selected_ids: Set<UInt64> = []
    public private(set) var checked_ids: Set<UInt64> = []
    public private(set) var designating: Bool = false
    public private(set) var number_error: String = ""
    @ObservationIgnored private let engine: ENGINE
    private var all_rows: [ITEM_ROW] = []
    private var preview: PREVIEW = PREVIEW(rows: [], changed: 0, runnable: 0, warning: 0,
        error: 0, conflict: 0)
    private var number_value: String = ""
    @ObservationIgnored private var check_anchor_id: UInt64 = 0

    public init(engine: ENGINE)
    {
        self.engine = engine
    }

    public var filter: VIEW_FILTER = .all
    {
        didSet {
            if filter != oldValue {
                apply_filter()
            }
        }
    }

    public var shows_empty_list: Bool
    {
        return all_rows.isEmpty
    }

    public var shows_empty_filter: Bool
    {
        return !all_rows.isEmpty && rows.isEmpty
    }

    public var empty_filter_title: String
    {
        return JOSA_FORMATTER.format(STRINGS.empty_filter_title,
            ITEM_LIST_VIEW_MODEL.filter_name(filter))
    }

    public var filter_label: String
    {
        return ITEM_LIST_VIEW_MODEL.filter_name(filter)
    }

    public var count_all: Int
    {
        return all_rows.count
    }

    public var count_changed: UInt32
    {
        return preview.changed
    }

    public var count_warning: UInt32
    {
        return preview.warning
    }

    public var count_error: UInt32
    {
        return preview.error
    }

    public var count_conflict: UInt32
    {
        return preview.conflict
    }

    public var can_drag: Bool
    {
        return filter == .all
    }

    public var header_text: String
    {
        var parts: [String] = [JOSA_FORMATTER.format(STRINGS.list_count, all_rows.count)]
        if designating {
            parts.append(JOSA_FORMATTER.format(STRINGS.list_checked, live_checked().count))
        } else {
            parts.append(JOSA_FORMATTER.format(STRINGS.list_selected, selected_ids.count))
        }
        if filter != .all {
            parts.append(JOSA_FORMATTER.format(STRINGS.list_filtered,
                ITEM_LIST_VIEW_MODEL.filter_name(filter)))
        }
        return parts.joined(separator: STRINGS.list_separator)
    }

    public var check_all_state: Bool?
    {
        let count: Int = live_checked().count
        if count == 0 {
            return false
        }
        return count == all_rows.count ? true : nil
    }

    public var number_text: String
    {
        get {
            return number_value
        }
        set {
            apply_number_text(newValue)
        }
    }

    public static func filter_name(_ filter: VIEW_FILTER) -> String
    {
        switch filter {
        case .all:
            return STRINGS.filter_all
        case .changed:
            return STRINGS.filter_changed
        case .warning:
            return STRINGS.filter_warning
        case .error:
            return STRINGS.filter_error
        case .conflict:
            return STRINGS.filter_conflict
        }
    }

    public func load(_ preview: PREVIEW)
    {
        self.preview = preview
        var loaded: [ITEM_ROW] = []
        loaded.reserveCapacity(preview.rows.count)
        for (offset, source) in preview.rows.enumerated() {
            let row = ITEM_ROW(source, index: UInt32(offset + 1))
            row.is_checked = checked_ids.contains(row.item_id)
            row.is_dimmed = designating && !row.is_checked
            loaded.append(row)
        }
        all_rows = loaded
        let live: Set<UInt64> = Set(loaded.map { $0.item_id })
        selected_ids.formIntersection(live)
        if designating {
            write_number_text()
        }
        apply_filter()
        selection_requested?(selected_ids)
    }

    public func set_selection(_ ids: some Sequence<UInt64>)
    {
        selected_ids = Set(ids)
    }

    public func clear_selection()
    {
        set_selection([])
        selection_requested?(selected_ids)
    }

    public func reveal(_ item_id: UInt64)
    {
        if !rows.contains(where: { $0.item_id == item_id }) {
            filter = .all
        }
        set_selection([item_id])
        selection_requested?(selected_ids)
        reveal_requested?(item_id)
    }

    public func drag_ids(_ grabbed_id: UInt64) -> [UInt64]
    {
        if !selected_ids.contains(grabbed_id) {
            return [grabbed_id]
        }
        return all_rows.filter { selected_ids.contains($0.item_id) }.map { $0.item_id }
    }

    public func can_drop(_ ids: [UInt64], target_id: UInt64, after: Bool) -> Bool
    {
        if !can_drag || ids.contains(target_id) {
            return false
        }
        return (try? engine.items_move(ids, target_id: target_id, after: after, dry_run: true))
            ?? false
    }

    public func drop(_ ids: [UInt64], target_id: UInt64, after: Bool) throws(ENGINE_ERROR)
        -> Bool
    {
        if !can_drop(ids, target_id: target_id, after: after) {
            return false
        }
        return try engine.items_move(ids, target_id: target_id, after: after, dry_run: false)
    }

    public func begin_designation(_ ids: some Sequence<UInt64>)
    {
        designating = true
        checked_ids = Set(ids)
        check_anchor_id = 0
        number_error = ""
        refresh_checks()
    }

    public func end_designation()
    {
        designating = false
        checked_ids = []
        number_error = ""
        number_value = ""
        refresh_checks()
    }

    public func toggle_check(_ item_id: UInt64, range: Bool)
    {
        let state: Bool = !checked_ids.contains(item_id)
        var ids: [UInt64] = [item_id]
        if range && check_anchor_id != 0,
            let from = rows.firstIndex(where: { $0.item_id == check_anchor_id }),
            let to = rows.firstIndex(where: { $0.item_id == item_id }) {
            ids = rows[min(from, to)...max(from, to)].map { $0.item_id }
        }
        for id in ids {
            if state {
                checked_ids.insert(id)
            } else {
                checked_ids.remove(id)
            }
        }
        check_anchor_id = item_id
        number_error = ""
        refresh_checks()
    }

    public func check_all(_ state: Bool)
    {
        checked_ids = state ? Set(all_rows.map { $0.item_id }) : []
        number_error = ""
        refresh_checks()
    }

    public func live_checked_ids() -> [UInt64]
    {
        return live_checked().map { $0.item_id }
    }

    public func index_of(_ item_id: UInt64) -> UInt32
    {
        guard let index = all_rows.firstIndex(where: { $0.item_id == item_id }) else {
            return 0
        }
        return UInt32(index + 1)
    }

    private func apply_number_text(_ text: String)
    {
        number_value = text
        let count = UInt32(all_rows.count)
        guard let list = try? ENGINE.index_list_parse(text, item_count: count) else {
            number_error = MESSAGES.index_error_text(.syntax, item_count: count)
            return
        }
        if list.error != .none {
            number_error = MESSAGES.index_error_text(list.error, item_count: count)
            return
        }
        number_error = ""
        checked_ids.subtract(all_rows.map { $0.item_id })
        checked_ids.formUnion(list.indices.map { all_rows[Int($0) - 1].item_id })
        update_checks()
    }

    private func refresh_checks()
    {
        update_checks()
        write_number_text()
    }

    private func update_checks()
    {
        for row in all_rows {
            row.is_checked = checked_ids.contains(row.item_id)
            row.is_dimmed = designating && !row.is_checked
        }
    }

    private func write_number_text()
    {
        let indices: [UInt32] = live_checked().map { $0.index }
        number_value = designating ? ((try? ENGINE.index_list_format(indices)) ?? "") : ""
    }

    private func live_checked() -> [ITEM_ROW]
    {
        return all_rows.filter { checked_ids.contains($0.item_id) }
    }

    private func apply_filter()
    {
        if filter == .all {
            rows = all_rows
        } else {
            rows = all_rows.filter { ITEM_LIST_VIEW_MODEL.matches($0, filter) }
        }
    }

    private static func matches(_ row: ITEM_ROW, _ filter: VIEW_FILTER) -> Bool
    {
        switch filter {
        case .all:
            return true
        case .changed:
            return row.changed
        case .warning:
            return row.status == .warning
        case .error:
            return row.status == .error
        case .conflict:
            return row.status == .conflict
        }
    }
}
