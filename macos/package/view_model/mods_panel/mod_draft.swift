struct MOD_DRAFT
{
    static let DATE_FORMATS: [String] =
        ["yyyyMMdd", "yyyy-MM-dd", "yyyy_MM_dd", "yyMMdd", "yyyyMMdd_HHmmss"]
    private static let NUMBER_LIMIT: Int32 = 1_000_000_000

    var kind: MOD_KIND = .insert
    var part: PART = .name
    var position_type: POSITION_TYPE = .start
    var n: UInt32 = 1
    var count: UInt32 = 1
    var find_text: String = ""
    var case_sensitive: Bool = false
    var occurrence: OCCURRENCE = .first
    var occurrence_n: UInt32 = 1
    var from_beginning: Bool = false
    var start_text: String = ""
    var to_end: Bool = false
    var end_text: String = ""
    var include_delimiters: Bool = false
    var value: [VALUE_PIECE] = []
    var number_start: Int32 = 1
    var number_step: Int32 = 1
    var number_digits: UInt32 = 3
    var modified_date_choice: String = "yyyyMMdd"
    var modified_date_custom: String = ""
    var created_date_choice: String = "yyyyMMdd"
    var created_date_custom: String = ""
    var size_unit: SIZE_UNIT = .auto
    var format_type: FORMAT_TYPE = .upper
    var format_digits: UInt32 = 3

    init()
    {
    }

    init(_ mod: MOD)
    {
        let position: POSITION = mod.position
        let number: NUMBER_SETTINGS = mod.tokens?.number ?? NUMBER_SETTINGS(digits: 3)
        kind = mod.kind
        part = mod.part
        position_type = position.type
        n = max(1, position.n ?? 1)
        count = max(1, position.count ?? 1)
        find_text = position.text ?? ""
        case_sensitive = position.case_sensitive ?? false
        occurrence = position.occurrence ?? .first
        occurrence_n = max(1, position.occurrence_n ?? 1)
        from_beginning = position.from_beginning ?? false
        start_text = position.start_text ?? ""
        to_end = position.to_end ?? false
        end_text = position.end_text ?? ""
        include_delimiters = position.include_delimiters ?? false
        value = MOD_DRAFT.normalize(mod.value ?? [])
        number_start = MOD_DRAFT.clamp_number(number.start)
        number_step = MOD_DRAFT.clamp_number(number.step)
        number_digits = MOD_DRAFT.clamp_digits(number.digits)
        (modified_date_choice, modified_date_custom) =
            MOD_DRAFT.split_date(mod.tokens?.modified_date?.format)
        (created_date_choice, created_date_custom) =
            MOD_DRAFT.split_date(mod.tokens?.created_date?.format)
        size_unit = mod.tokens?.size?.unit ?? .auto
        format_type = mod.format?.type ?? .upper
        format_digits = MOD_DRAFT.clamp_digits(mod.format?.digits ?? 3)
    }

    func mod(scope: SCOPE, targets: [UInt64]?) -> MOD
    {
        let has_value: Bool = kind == .insert || kind == .replace
        let position = POSITION(type: position_type, n: n, count: count, text: find_text,
            case_sensitive: case_sensitive, occurrence: occurrence, occurrence_n: occurrence_n,
            from_beginning: from_beginning, start_text: start_text, to_end: to_end,
            end_text: end_text, include_delimiters: include_delimiters)
        let tokens = TOKEN_SETTINGS(
            number: NUMBER_SETTINGS(start: number_start, step: number_step,
                digits: number_digits),
            modified_date: DATE_SETTINGS(
                format: MOD_DRAFT.date_format(modified_date_choice, modified_date_custom)),
            created_date: DATE_SETTINGS(
                format: MOD_DRAFT.date_format(created_date_choice, created_date_custom)),
            size: SIZE_SETTINGS(unit: size_unit))
        let format = FORMAT_SETTINGS(type: format_type,
            digits: format_type == .pad_digits ? format_digits : nil)
        return MOD(kind: kind, part: part, position: position, value: has_value ? value : nil,
            tokens: has_value ? tokens : nil, format: kind == .format ? format : nil,
            scope: scope, targets: targets)
    }

    static func clamp_number(_ value: Int32) -> Int32
    {
        return min(max(value, -NUMBER_LIMIT), NUMBER_LIMIT)
    }

    static func clamp_digits(_ value: UInt32) -> UInt32
    {
        return min(max(value, 1), 32)
    }

    static func normalize(_ pieces: [VALUE_PIECE]) -> [VALUE_PIECE]
    {
        var result: [VALUE_PIECE] = []
        for piece in pieces {
            if let token = piece.token {
                result.append(VALUE_PIECE(token: token))
            } else if let text = piece.text, !text.isEmpty {
                if let last = result.last, last.token == nil {
                    result[result.count - 1] = VALUE_PIECE(text: (last.text ?? "") + text)
                } else {
                    result.append(VALUE_PIECE(text: text))
                }
            }
        }
        return result
    }

    private static func split_date(_ format: String?) -> (choice: String, custom: String)
    {
        let text: String = format ?? "yyyyMMdd"
        if DATE_FORMATS.contains(text) {
            return (text, "")
        }
        return ("", text)
    }

    private static func date_format(_ choice: String, _ custom: String) -> String
    {
        return choice == "" ? custom : choice
    }
}
