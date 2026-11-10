import Foundation
import Observation

@MainActor
@Observable
public final class STATUS_BAR_VIEW_MODEL
{
    private static let PROGRESS_DELAY: TimeInterval = 0.3
    private static let TIME_FORMATTER: DateFormatter = make_time_formatter()

    public private(set) var message: String = ""
    public private(set) var time_text: String = ""
    public private(set) var progress_label: String = ""
    public private(set) var progress_visible: Bool = false
    public private(set) var progress_text: String = ""
    public private(set) var progress_value: Double = 0
    @ObservationIgnored private let list: ITEM_LIST_VIEW_MODEL
    @ObservationIgnored private let now: () -> Date
    @ObservationIgnored private var progress: PROGRESS? = nil
    @ObservationIgnored private var progress_start: Date = .distantPast

    public init(list: ITEM_LIST_VIEW_MODEL, now: @escaping () -> Date)
    {
        self.list = list
        self.now = now
    }

    public var error_text: String
    {
        return JOSA_FORMATTER.format(STRINGS.pill_error, list.count_error)
    }

    public var conflict_text: String
    {
        return JOSA_FORMATTER.format(STRINGS.pill_conflict, list.count_conflict)
    }

    public var warning_text: String
    {
        return JOSA_FORMATTER.format(STRINGS.pill_warning, list.count_warning)
    }

    public var has_errors: Bool
    {
        return list.count_error > 0
    }

    public var has_conflicts: Bool
    {
        return list.count_conflict > 0
    }

    public var has_warnings: Bool
    {
        return list.count_warning > 0
    }

    public var active_filter: VIEW_FILTER
    {
        return list.filter
    }

    public func toggle_error()
    {
        toggle(.error)
    }

    public func toggle_conflict()
    {
        toggle(.conflict)
    }

    public func toggle_warning()
    {
        toggle(.warning)
    }

    public func post(_ text: String)
    {
        message = text
        time_text = STATUS_BAR_VIEW_MODEL.TIME_FORMATTER.string(from: now())
    }

    public func begin_progress(_ progress: PROGRESS, label: String)
    {
        self.progress = progress
        progress_start = now()
        progress_label = label
        progress_visible = false
    }

    public func poll_progress()
    {
        guard let progress else {
            return
        }
        let (done, total): (UInt32, UInt32) = progress.read()
        progress_text = JOSA_FORMATTER.format(STRINGS.progress_count,
            STATUS_BAR_VIEW_MODEL.grouped(done), STATUS_BAR_VIEW_MODEL.grouped(total))
        progress_value = total == 0 ? 0 : Double(done) / Double(total)
        if now().timeIntervalSince(progress_start) >= STATUS_BAR_VIEW_MODEL.PROGRESS_DELAY {
            progress_visible = true
        }
    }

    public func end_progress()
    {
        progress = nil
        progress_visible = false
        progress_label = ""
        progress_text = ""
        progress_value = 0
    }

    private func toggle(_ filter: VIEW_FILTER)
    {
        list.filter = list.filter == filter ? .all : filter
    }

    private static func grouped(_ value: UInt32) -> String
    {
        let digits: [Character] = Array(String(value))
        var text: String = ""
        for (offset, digit) in digits.enumerated() {
            if offset > 0 && (digits.count - offset) % 3 == 0 {
                text.append(",")
            }
            text.append(digit)
        }
        return text
    }

    private static func make_time_formatter() -> DateFormatter
    {
        let formatter = DateFormatter()
        formatter.locale = Locale(identifier: "en_US_POSIX")
        formatter.dateFormat = "HH:mm"
        return formatter
    }
}
