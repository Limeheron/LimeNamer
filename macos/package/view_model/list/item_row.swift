import Foundation
import Observation

@MainActor
@Observable
public final class ITEM_ROW
{
    private static let DATE_FORMATTER: DateFormatter = make_date_formatter()

    public let item_id: UInt64
    public let index: UInt32
    public let folder: String
    public let name: String
    public let new_name: String
    public let status: STATUS
    public let changed: Bool
    public let direct: Bool
    public let is_dir: Bool
    public internal(set) var is_checked: Bool = false
    public internal(set) var is_dimmed: Bool = false
    private let source: ROW

    public init(_ row: ROW, index: UInt32)
    {
        item_id = row.item_id
        self.index = index
        folder = row.folder
        name = row.name
        new_name = row.new_name
        status = row.status
        changed = row.changed
        direct = row.direct
        is_dir = row.is_dir
        source = row
    }

    public var full_path: String
    {
        return (folder as NSString).appendingPathComponent(name)
    }

    public var new_name_parts: (head: String, mark: String, tail: String)
    {
        return ITEM_ROW.split(new_name, prefix_len: source.prefix_len,
            suffix_len: source.suffix_len)
    }

    public var status_text: String
    {
        return MESSAGES.status_name(status)
    }

    public var problems: [String]
    {
        return MESSAGES.row_messages(source)
    }

    public var problem_text: String
    {
        return problems.joined(separator: "\n")
    }

    public var has_problems: Bool
    {
        return source.errors != .none || source.warnings != .none || source.conflict != .none
            || source.adjusted
    }

    public var modified_text: String
    {
        return ITEM_ROW.date_text(source.modified_ms)
    }

    public var created_text: String
    {
        return ITEM_ROW.date_text(source.created_ms)
    }

    public var size_text: String
    {
        return is_dir ? STRINGS.size_none : ITEM_ROW.size_of(source.size)
    }

    static func split(_ text: String, prefix_len: UInt32, suffix_len: UInt32)
        -> (head: String, mark: String, tail: String)
    {
        let scalars: String.UnicodeScalarView = text.unicodeScalars
        let head_end: String.Index = scalars.index(scalars.startIndex,
            offsetBy: Int(prefix_len), limitedBy: scalars.endIndex) ?? scalars.endIndex
        var tail_start: String.Index = scalars.index(scalars.endIndex,
            offsetBy: -Int(suffix_len), limitedBy: scalars.startIndex) ?? scalars.startIndex
        if tail_start < head_end {
            tail_start = head_end
        }
        return (String(scalars[..<head_end]), String(scalars[head_end..<tail_start]),
            String(scalars[tail_start...]))
    }

    static func size_of(_ bytes: Int64) -> String
    {
        let kb: Double = 1024
        let value: Double = Double(bytes)
        if value < kb {
            return String(bytes) + "B"
        }
        if value < kb * kb {
            return String(Int64((value / kb).rounded(.toNearestOrEven))) + "KB"
        }
        if value < kb * kb * kb {
            return String(format: "%.1f", value / (kb * kb)) + "MB"
        }
        return String(format: "%.1f", value / (kb * kb * kb)) + "GB"
    }

    private static func date_text(_ milliseconds: Int64) -> String
    {
        if milliseconds <= 0 {
            return ""
        }
        return DATE_FORMATTER.string(from: Date(timeIntervalSince1970: Double(milliseconds) / 1000))
    }

    private static func make_date_formatter() -> DateFormatter
    {
        let formatter = DateFormatter()
        formatter.locale = Locale(identifier: "en_US_POSIX")
        formatter.calendar = Calendar(identifier: .gregorian)
        formatter.dateFormat = "yyyy-MM-dd HH:mm"
        return formatter
    }
}
