import Observation

@MainActor
@Observable
public final class MOD_LIST_VIEW_MODEL
{
    private static let KINDS: [MOD_KIND] = [.insert, .delete, .replace, .format]

    @ObservationIgnored public var changed: ((String) -> Void)? = nil
    @ObservationIgnored public var notice: ((String) -> Void)? = nil
    public let editor: MOD_EDITOR_VIEW_MODEL
    public private(set) var page: MOD_PAGE = .list
    public private(set) var cards: [MOD_CARD] = []
    public var add_part: PART = .name
    @ObservationIgnored private let engine: ENGINE

    public init(engine: ENGINE, list: ITEM_LIST_VIEW_MODEL)
    {
        self.engine = engine
        editor = MOD_EDITOR_VIEW_MODEL(engine: engine, list: list)
        editor.notice = { [weak self] text in
            self?.notice?(text)
        }
    }

    public var is_editing: Bool
    {
        return page == .edit
    }

    public var has_mods: Bool
    {
        return !cards.isEmpty
    }

    public var all_enabled_state: Bool?
    {
        let enabled: Int = enabled_count
        if enabled == 0 {
            return false
        }
        return enabled == cards.count ? true : nil
    }

    public var enabled_count: Int
    {
        return cards.filter { $0.enabled }.count
    }

    public var kind_cards: [KIND_CARD]
    {
        return MOD_LIST_VIEW_MODEL.KINDS.map { MOD_LIST_VIEW_MODEL.kind_card($0, add_part) }
    }

    public var back_label: String
    {
        return editor.is_new ? STRINGS.back_to_add : STRINGS.back_to_list
    }

    public func load(_ preview: PREVIEW) throws(ENGINE_ERROR)
    {
        var positions: [UInt64: UInt32] = [:]
        for (offset, row) in preview.rows.enumerated() {
            positions[row.item_id] = UInt32(offset + 1)
        }

        let json: String = try engine.mods_get()
        guard let mods = try? MOD_JSON.parse_list(json) else {
            throw ENGINE_ERROR(result: .internal_error)
        }
        var loaded: [MOD_CARD] = []
        for mod in mods {
            let targets: [UInt64] = mod.targets ?? []
            let live = UInt32(targets.filter { positions[$0] != nil }.count)
            var direct_index: UInt32 = 0
            if mod.direct, let first = targets.first {
                direct_index = positions[first] ?? 0
            }
            loaded.append(MOD_CARD(id: mod.id ?? 0, enabled: mod.enabled ?? true,
                direct: mod.direct,
                summary: MOD_SUMMARY.describe(mod, direct_index: direct_index,
                    target_count: live),
                mod: mod))
        }
        cards = loaded
        if page == .edit {
            editor.refresh()
        }
    }

    public func open_add()
    {
        add_part = .name
        page = .add
    }

    public func close_add()
    {
        page = .list
    }

    public func choose_kind(_ kind: MOD_KIND)
    {
        editor.begin_new(kind, part: add_part)
        page = .edit
    }

    public func open_card(_ card: MOD_CARD)
    {
        editor.begin_edit(card.mod)
        page = .edit
    }

    public func apply() throws(ENGINE_ERROR) -> Bool
    {
        let was_new: Bool = editor.is_new
        let applied: Bool = try editor.apply()
        if !applied {
            return false
        }
        page = .list
        changed?(was_new ? STRINGS.mod_added : STRINGS.mod_updated)
        return true
    }

    public func go_back()
    {
        if page == .edit && editor.is_new {
            let part: PART = editor.part
            _ = editor.cancel()
            add_part = part
            page = .add
            return
        }
        if page == .edit {
            cancel()
            return
        }
        close_add()
    }

    public func cancel()
    {
        let text: String = editor.cancel()
        page = .list
        if text != "" {
            notice?(text)
        }
    }

    public func set_enabled(_ card: MOD_CARD, enabled: Bool) throws(ENGINE_ERROR)
    {
        try engine.mod_set_enabled(card.id, enabled: enabled)
        changed?(enabled ? STRINGS.mod_enabled : STRINGS.mod_disabled)
    }

    public func toggle_all() throws(ENGINE_ERROR)
    {
        let enable: Bool = all_enabled_state != true
        try engine.mods_set_all_enabled(enable)
        changed?(enable ? STRINGS.mods_all_enabled : STRINGS.mods_all_disabled)
    }

    public func remove(_ card: MOD_CARD) throws(ENGINE_ERROR)
    {
        try engine.mod_remove(card.id)
        changed?(STRINGS.mod_removed)
    }

    public func can_drop(_ id: UInt64, target_id: UInt64, after: Bool) -> Bool
    {
        if id == target_id {
            return false
        }
        return (try? engine.mod_move(id, target_id: target_id, after: after, dry_run: true))
            ?? false
    }

    public func drop(_ id: UInt64, target_id: UInt64, after: Bool) throws(ENGINE_ERROR) -> Bool
    {
        if !can_drop(id, target_id: target_id, after: after) {
            return false
        }
        _ = try engine.mod_move(id, target_id: target_id, after: after, dry_run: false)
        changed?(STRINGS.mod_moved)
        return true
    }

    private static func kind_card(_ kind: MOD_KIND, _ part: PART) -> KIND_CARD
    {
        let name: String = "kind_\(kind.rawValue)"
        let example: String = "example_\(part.rawValue)_\(kind.rawValue)"
        return KIND_CARD(kind: kind, name: STRINGS.text(name),
            description: STRINGS.text(name + "_desc"),
            example_before: STRINGS.text(example + "_before"),
            example_after: STRINGS.text(example + "_after"))
    }
}
