@MainActor
final class POSITION_SAMPLES
{
    private static let INSERT_VALUE: String = "2024_"
    private static let REPLACE_VALUE: String = "제주_"
    private static let BEFORE_ANCHOR: String = "0001"
    private static let AFTER_ANCHOR: String = "IMG"
    private static let RANGE_TEXT: String = "IMG_"
    private static let BETWEEN_START: String = "IMG"
    private static let INSERT_TYPES: [POSITION_TYPE] = [.start, .end, .after_nth,
        .before_nth_from_end, .before_text, .after_text]
    private static let RANGE_TYPES: [POSITION_TYPE] = [.all, .from_nth, .first_n, .last_n,
        .text, .between]

    private let engine: ENGINE
    private var cache: [MOD_KIND: [POSITION_CHOICE]] = [:]

    init(engine: ENGINE)
    {
        self.engine = engine
    }

    static func default_type(_ kind: MOD_KIND) -> POSITION_TYPE
    {
        return kind == .insert ? .start : .all
    }

    static func label(_ type: POSITION_TYPE) -> String
    {
        switch type {
        case .start:
            return STRINGS.position_start
        case .end:
            return STRINGS.position_end
        case .after_nth:
            return STRINGS.position_short_after_nth
        case .before_nth_from_end:
            return STRINGS.position_short_before_nth_from_end
        case .before_text:
            return STRINGS.position_short_before_text
        case .after_text:
            return STRINGS.position_short_after_text
        case .all:
            return STRINGS.position_all
        case .from_nth:
            return STRINGS.position_short_from_nth
        case .first_n:
            return STRINGS.position_first_n
        case .last_n:
            return STRINGS.position_last_n
        case .text:
            return STRINGS.position_text
        case .between:
            return STRINGS.position_between
        }
    }

    func choices(_ kind: MOD_KIND) -> [POSITION_CHOICE]
    {
        if let found = cache[kind] {
            return found
        }
        let types: [POSITION_TYPE] = kind == .insert ? POSITION_SAMPLES.INSERT_TYPES
            : POSITION_SAMPLES.RANGE_TYPES
        let result: [POSITION_CHOICE] = types.map { make(kind, $0) }
        cache[kind] = result
        return result
    }

    private func make(_ kind: MOD_KIND, _ type: POSITION_TYPE) -> POSITION_CHOICE
    {
        let label: String = POSITION_SAMPLES.label(type)
        let description: String = STRINGS.text("position_desc_\(type.rawValue)")
        if kind == .insert {
            let inserted = POSITION_SAMPLES.parts(sample(kind, type), after: true)
            let after: [SAMPLE_PIECE] = POSITION_SAMPLES.pieces([(inserted.head, .plain),
                (inserted.mark, .mark), (inserted.tail, .plain)])
            return POSITION_CHOICE(type: type, label: label, description: description,
                before: POSITION_SAMPLES.pieces([(inserted.head + inserted.tail, .plain)]),
                after: after,
                tile: POSITION_SAMPLES.insert_tile(type, inserted.head, inserted.tail))
        }
        let range = POSITION_SAMPLES.parts(sample(.delete, type), after: false)
        let tile: [SAMPLE_PIECE] = POSITION_SAMPLES.range_tile(type, range)
        if kind == .format {
            return POSITION_CHOICE(type: type, label: label, description: description,
                before: tile, after: [], tile: tile)
        }
        let before: [SAMPLE_PIECE] = POSITION_SAMPLES.pieces([(range.head, .plain),
            (range.mark, .removed), (range.tail, .plain)])
        let changed: [SAMPLE_PIECE] = kind == .replace
            ? POSITION_SAMPLES.pieces([(range.head, .plain),
                (POSITION_SAMPLES.REPLACE_VALUE, .mark), (range.tail, .plain)])
            : POSITION_SAMPLES.pieces([(range.head + range.tail, .plain)])
        return POSITION_CHOICE(type: type, label: label, description: description,
            before: before, after: changed, tile: tile)
    }

    private func sample(_ kind: MOD_KIND, _ type: POSITION_TYPE) -> EXAMPLE?
    {
        let value: [VALUE_PIECE]? = kind == .insert
            ? [VALUE_PIECE(text: POSITION_SAMPLES.INSERT_VALUE)] : nil
        let mod: MOD = MOD(kind: kind, part: .name, position: POSITION_SAMPLES.position(type),
            value: value)
        return try? engine.mod_sample(MOD_JSON.write(mod))
    }

    private static func position(_ type: POSITION_TYPE) -> POSITION
    {
        switch type {
        case .after_nth:
            return POSITION(type: type, n: 3)
        case .before_nth_from_end:
            return POSITION(type: type, n: 4)
        case .before_text:
            return POSITION(type: type, text: BEFORE_ANCHOR)
        case .after_text:
            return POSITION(type: type, text: AFTER_ANCHOR)
        case .first_n, .last_n:
            return POSITION(type: type, n: 4)
        case .from_nth:
            return POSITION(type: type, n: 2, count: 3)
        case .text:
            return POSITION(type: type, text: RANGE_TEXT)
        case .between:
            return POSITION(type: type, start_text: BETWEEN_START, to_end: true)
        default:
            return POSITION(type: type)
        }
    }

    private static func parts(_ example: EXAMPLE?, after: Bool)
        -> (head: String, mark: String, tail: String)
    {
        guard let example else {
            return ("", "", "")
        }
        return ITEM_ROW.split(after ? example.after : example.before,
            prefix_len: example.prefix_len, suffix_len: example.suffix_len)
    }

    private static func insert_tile(_ type: POSITION_TYPE, _ head: String, _ tail: String)
        -> [SAMPLE_PIECE]
    {
        if type == .after_text && head.hasSuffix(AFTER_ANCHOR) {
            return pieces([(String(head.dropLast(AFTER_ANCHOR.count)), .plain),
                (AFTER_ANCHOR, .anchor), ("", .caret), (tail, .plain)])
        }
        if type == .before_text && tail.hasPrefix(BEFORE_ANCHOR) {
            return pieces([(head, .plain), ("", .caret), (BEFORE_ANCHOR, .anchor),
                (String(tail.dropFirst(BEFORE_ANCHOR.count)), .plain)])
        }
        return pieces([(head, .plain), ("", .caret), (tail, .plain)])
    }

    private static func range_tile(_ type: POSITION_TYPE,
        _ range: (head: String, mark: String, tail: String)) -> [SAMPLE_PIECE]
    {
        let start: String = BETWEEN_START
        if type == .text {
            return pieces([(range.head, .plain), (range.mark, .mark_anchor),
                (range.tail, .plain)])
        }
        if type == .between && range.head.hasSuffix(start) {
            return pieces([(String(range.head.dropLast(start.count)), .plain), (start, .anchor),
                (range.mark, .mark), (range.tail, .plain)])
        }
        return pieces([(range.head, .plain), (range.mark, .mark), (range.tail, .plain)])
    }

    private static func pieces(_ parts: [(String, SAMPLE_STYLE)]) -> [SAMPLE_PIECE]
    {
        return parts.compactMap { part in
            if part.0.isEmpty && part.1 != .caret {
                return nil
            }
            return SAMPLE_PIECE(text: part.0, style: part.1)
        }
    }
}
