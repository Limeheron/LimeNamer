import Observation

@MainActor
@Observable
public final class MOD_EDITOR_VIEW_MODEL
{
    public static let PART_CHOICES: [CHOICE<PART>] = [
        CHOICE(value: .name, label: STRINGS.part_name),
        CHOICE(value: .ext, label: STRINGS.part_ext),
        CHOICE(value: .full, label: STRINGS.part_full),
    ]
    public static let SCOPE_CHOICES: [CHOICE<SCOPE>] = [
        CHOICE(value: .all, label: STRINGS.scope_all),
        CHOICE(value: .items, label: STRINGS.scope_items),
    ]
    public static let FORMAT_CHOICES: [CHOICE<FORMAT_TYPE>] = [
        CHOICE(value: .upper, label: STRINGS.format_choice_upper),
        CHOICE(value: .lower, label: STRINGS.format_choice_lower),
        CHOICE(value: .title, label: STRINGS.format_choice_title),
        CHOICE(value: .sentence, label: STRINGS.format_choice_sentence),
        CHOICE(value: .pad_digits, label: STRINGS.format_choice_pad_digits),
    ]
    public static let SIZE_UNIT_CHOICES: [CHOICE<SIZE_UNIT>] = [
        CHOICE(value: .auto, label: STRINGS.size_unit_auto),
        CHOICE(value: .b, label: STRINGS.size_unit_b),
        CHOICE(value: .kb, label: STRINGS.size_unit_kb),
        CHOICE(value: .mb, label: STRINGS.size_unit_mb),
        CHOICE(value: .gb, label: STRINGS.size_unit_gb),
    ]
    public static let DATE_FORMAT_CHOICES: [CHOICE<String>] =
        MOD_DRAFT.DATE_FORMATS.map { CHOICE(value: $0, label: $0) }
        + [CHOICE(value: "", label: STRINGS.date_custom)]
    private static let OCCURRENCES: [CHOICE<OCCURRENCE>] = [
        CHOICE(value: .first, label: STRINGS.occurrence_choice_first),
        CHOICE(value: .last, label: STRINGS.occurrence_choice_last),
        CHOICE(value: .nth, label: STRINGS.occurrence_choice_nth),
        CHOICE(value: .nth_from_end, label: STRINGS.occurrence_choice_nth_from_end),
        CHOICE(value: .all, label: STRINGS.occurrence_choice_all),
    ]

    @ObservationIgnored public var notice: ((String) -> Void)? = nil
    @ObservationIgnored public var focus_requested: ((MOD_FIELD) -> Void)? = nil
    public private(set) var editing_id: UInt64 = 0
    public private(set) var summary: String = ""
    public private(set) var has_example: Bool = false
    public private(set) var example_index_text: String = ""
    public private(set) var example_state_text: String = ""
    public private(set) var example_before: (head: String, mark: String, tail: String) =
        ("", "", "")
    public private(set) var example_after: (head: String, mark: String, tail: String) =
        ("", "", "")
    @ObservationIgnored private let engine: ENGINE
    @ObservationIgnored private let list: ITEM_LIST_VIEW_MODEL
    @ObservationIgnored private let samples: POSITION_SAMPLES
    private var draft: MOD_DRAFT = MOD_DRAFT()
    private var scope_value: SCOPE = .all
    private var problems: [MOD_FIELD: String] = [:]
    private var active: Bool = false
    @ObservationIgnored private var tried: Bool = false
    @ObservationIgnored private var original: MOD = MOD()
    @ObservationIgnored private var last_targets: Set<UInt64> = []

    public init(engine: ENGINE, list: ITEM_LIST_VIEW_MODEL)
    {
        self.engine = engine
        self.list = list
        samples = POSITION_SAMPLES(engine: engine)
        list.checks_changed = { [weak self] in
            self?.on_list_changed()
        }
    }

    public var is_new: Bool
    {
        return editing_id == 0
    }

    public var is_active: Bool
    {
        return active
    }

    public var kind: MOD_KIND
    {
        return draft.kind
    }

    public var part: PART
    {
        get {
            return draft.part
        }
        set {
            change(\.part, newValue)
        }
    }

    public var position_type: POSITION_TYPE
    {
        get {
            return draft.position_type
        }
        set {
            if newValue == .between && draft.occurrence == .all {
                draft.occurrence = .first
            }
            change(\.position_type, newValue)
        }
    }

    public var n: UInt32
    {
        get {
            return draft.n
        }
        set {
            change(\.n, max(1, newValue))
        }
    }

    public var count: UInt32
    {
        get {
            return draft.count
        }
        set {
            change(\.count, max(1, newValue))
        }
    }

    public var find_text: String
    {
        get {
            return draft.find_text
        }
        set {
            change(\.find_text, newValue)
        }
    }

    public var case_sensitive: Bool
    {
        get {
            return draft.case_sensitive
        }
        set {
            change(\.case_sensitive, newValue)
        }
    }

    public var occurrence: OCCURRENCE
    {
        get {
            return draft.occurrence
        }
        set {
            change(\.occurrence, newValue)
        }
    }

    public var occurrence_n: UInt32
    {
        get {
            return draft.occurrence_n
        }
        set {
            change(\.occurrence_n, max(1, newValue))
        }
    }

    public var from_beginning: Bool
    {
        get {
            return draft.from_beginning
        }
        set {
            change(\.from_beginning, newValue)
        }
    }

    public var start_text: String
    {
        get {
            return draft.start_text
        }
        set {
            change(\.start_text, newValue)
        }
    }

    public var to_end: Bool
    {
        get {
            return draft.to_end
        }
        set {
            change(\.to_end, newValue)
        }
    }

    public var end_text: String
    {
        get {
            return draft.end_text
        }
        set {
            change(\.end_text, newValue)
        }
    }

    public var include_delimiters: Bool
    {
        get {
            return draft.include_delimiters
        }
        set {
            change(\.include_delimiters, newValue)
        }
    }

    public var value: [VALUE_PIECE]
    {
        get {
            return draft.value
        }
        set {
            change(\.value, MOD_DRAFT.normalize(newValue))
        }
    }

    public var number_start: Int32
    {
        get {
            return draft.number_start
        }
        set {
            change(\.number_start, MOD_DRAFT.clamp_number(newValue))
        }
    }

    public var number_step: Int32
    {
        get {
            return draft.number_step
        }
        set {
            change(\.number_step, MOD_DRAFT.clamp_number(newValue))
        }
    }

    public var number_digits: UInt32
    {
        get {
            return draft.number_digits
        }
        set {
            change(\.number_digits, MOD_DRAFT.clamp_digits(newValue))
        }
    }

    public var modified_date_choice: String
    {
        get {
            return draft.modified_date_choice
        }
        set {
            change(\.modified_date_choice, newValue)
        }
    }

    public var modified_date_custom: String
    {
        get {
            return draft.modified_date_custom
        }
        set {
            change(\.modified_date_custom, newValue)
        }
    }

    public var created_date_choice: String
    {
        get {
            return draft.created_date_choice
        }
        set {
            change(\.created_date_choice, newValue)
        }
    }

    public var created_date_custom: String
    {
        get {
            return draft.created_date_custom
        }
        set {
            change(\.created_date_custom, newValue)
        }
    }

    public var size_unit: SIZE_UNIT
    {
        get {
            return draft.size_unit
        }
        set {
            change(\.size_unit, newValue)
        }
    }

    public var format_type: FORMAT_TYPE
    {
        get {
            return draft.format_type
        }
        set {
            change(\.format_type, newValue)
        }
    }

    public var format_digits: UInt32
    {
        get {
            return draft.format_digits
        }
        set {
            change(\.format_digits, MOD_DRAFT.clamp_digits(newValue))
        }
    }

    public var scope: SCOPE
    {
        get {
            return scope_value
        }
        set {
            if newValue != scope_value {
                apply_scope(newValue)
                refresh()
            }
        }
    }

    public var position_tiles: [POSITION_CHOICE]
    {
        return samples.choices(draft.kind)
    }

    public var occurrence_choices: [CHOICE<OCCURRENCE>]
    {
        if draft.position_type == .between {
            return Array(MOD_EDITOR_VIEW_MODEL.OCCURRENCES.prefix(4))
        }
        return MOD_EDITOR_VIEW_MODEL.OCCURRENCES
    }

    public var shows_n: Bool
    {
        switch draft.position_type {
        case .after_nth, .before_nth_from_end, .first_n, .last_n, .from_nth:
            return true
        default:
            return false
        }
    }

    public var shows_count: Bool
    {
        return draft.position_type == .from_nth
    }

    public var shows_text: Bool
    {
        switch draft.position_type {
        case .before_text, .after_text, .text:
            return true
        default:
            return false
        }
    }

    public var shows_occurrence: Bool
    {
        return shows_text || shows_between
    }

    public var shows_occurrence_n: Bool
    {
        return shows_occurrence && (draft.occurrence == .nth || draft.occurrence == .nth_from_end)
    }

    public var shows_between: Bool
    {
        return draft.position_type == .between
    }

    public var shows_start_text: Bool
    {
        return shows_between && !draft.from_beginning
    }

    public var shows_end_text: Bool
    {
        return shows_between && !draft.to_end
    }

    public var value_label: String
    {
        return draft.kind == .insert ? STRINGS.field_value_insert : STRINGS.field_value_replace
    }

    public var shows_value: Bool
    {
        return draft.kind == .insert || draft.kind == .replace
    }

    public var shows_format: Bool
    {
        return draft.kind == .format
    }

    public var shows_value_or_format: Bool
    {
        return shows_value || shows_format
    }

    public var shows_digits: Bool
    {
        return shows_format && draft.format_type == .pad_digits
    }

    public var uses_number: Bool
    {
        return uses(.number)
    }

    public var uses_modified_date: Bool
    {
        return uses(.modified_date)
    }

    public var uses_created_date: Bool
    {
        return uses(.created_date)
    }

    public var uses_size: Bool
    {
        return uses(.size)
    }

    public var shows_modified_date_custom: Bool
    {
        return uses_modified_date && draft.modified_date_choice == ""
    }

    public var shows_created_date_custom: Bool
    {
        return uses_created_date && draft.created_date_choice == ""
    }

    public static func part_tip(_ part: PART) -> String
    {
        switch part {
        case .name:
            return STRINGS.part_tip_name
        case .ext:
            return STRINGS.part_tip_ext
        case .full:
            return STRINGS.part_tip_full
        }
    }

    public func problem(_ field: MOD_FIELD) -> String
    {
        return problems[field] ?? ""
    }

    public func position_choices(_ kind: MOD_KIND) -> [POSITION_CHOICE]
    {
        return samples.choices(kind)
    }

    public func begin_new(_ kind: MOD_KIND, position: POSITION_TYPE)
    {
        begin(MOD(kind: kind, part: .name, position: POSITION(type: position)), id: 0)
    }

    public func restart(_ kind: MOD_KIND, position: POSITION_TYPE)
    {
        let saved: MOD = original
        begin(MOD(kind: kind, part: .name, position: POSITION(type: position)), id: editing_id)
        original = saved
    }

    public func begin_edit(_ mod: MOD)
    {
        begin(mod, id: mod.id ?? 0)
    }

    public func apply() throws(ENGINE_ERROR) -> Bool
    {
        tried = true
        if !(try check()) {
            let fields: [MOD_FIELD] = problems.keys.sorted { $0.rawValue < $1.rawValue }
            var text: String = JOSA_FORMATTER.format(STRINGS.apply_failed,
                problems[fields[0]] ?? "")
            if fields.count > 1 {
                text += JOSA_FORMATTER.format(STRINGS.apply_failed_more, fields.count)
            }
            notice?(text)
            focus_requested?(fields[0])
            return false
        }

        let json: String = MOD_JSON.write(build())
        if editing_id == 0 {
            _ = try engine.mod_add(json)
        } else {
            try engine.mod_update(editing_id, json: json)
        }
        finish()
        return true
    }

    public func cancel() -> String
    {
        let dirty: Bool = build() != original
        let was_new: Bool = is_new
        finish()
        if was_new {
            return STRINGS.draft_discarded_new
        }
        return dirty ? STRINGS.draft_discarded : ""
    }

    public func refresh()
    {
        summary = MOD_SUMMARY.describe(build(), direct_index: 0,
            target_count: target_count())
        refresh_example()
        if tried {
            _ = try? check()
        }
    }

    private func begin(_ mod: MOD, id: UInt64)
    {
        active = false
        editing_id = id
        draft = MOD_DRAFT(mod)
        last_targets = Set(mod.targets ?? [])
        list.end_designation()
        scope_value = .all
        apply_scope(mod.scope)
        tried = false
        problems = [:]
        active = true
        original = build()
        refresh()
    }

    private func finish()
    {
        active = false
        list.end_designation()
        tried = false
        problems = [:]
    }

    private func change<T: Equatable>(_ field: WritableKeyPath<MOD_DRAFT, T>, _ value: T)
    {
        if draft[keyPath: field] == value {
            return
        }
        draft[keyPath: field] = value
        refresh()
    }

    private func apply_scope(_ value: SCOPE)
    {
        if value == scope_value {
            return
        }
        if value == .items {
            list.begin_designation(last_targets)
        } else {
            last_targets = list.checked_ids
            list.end_designation()
        }
        scope_value = value
    }

    private func on_list_changed()
    {
        if active && scope_value == .items {
            refresh()
        }
    }

    private func check() throws(ENGINE_ERROR) -> Bool
    {
        var found: [MOD_FIELD: String] = [:]
        for item in try engine.mod_check(MOD_JSON.write(build())) {
            found[item.field] = MESSAGES.problem_text(item, kind: draft.kind, direct: false)
        }
        if scope_value == .items && list.number_error != "" {
            found[.targets] = list.number_error
        }
        problems = found
        return found.isEmpty
    }

    private func refresh_example()
    {
        let example: EXAMPLE
        do {
            example = try engine.mod_example(MOD_JSON.write(build()), before_mod_id: editing_id)
        } catch {
            has_example = false
            return
        }
        example_index_text = example.item_id == 0
            ? STRINGS.example_default
            : JOSA_FORMATTER.format(STRINGS.example_index, example.index)
        example_state_text = MESSAGES.example_state_text(example.state)
        example_before = ITEM_ROW.split(example.before, prefix_len: example.prefix_len,
            suffix_len: example.suffix_len)
        example_after = ITEM_ROW.split(example.after, prefix_len: example.prefix_len,
            suffix_len: example.suffix_len)
        has_example = true
    }

    private func build() -> MOD
    {
        let targets: [UInt64]? = scope_value == .items ? list.live_checked_ids() : nil
        return draft.mod(scope: scope_value, targets: targets)
    }

    private func target_count() -> UInt32
    {
        return scope_value == .items ? UInt32(list.live_checked_ids().count) : 0
    }

    private func uses(_ token: TOKEN_KIND) -> Bool
    {
        return shows_value && draft.value.contains { $0.token == token }
    }
}
