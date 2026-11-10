import Foundation
import Testing

@testable import limenamer_view_model

@MainActor
struct STATUS_BAR_VIEW_MODEL_TESTS
{
    @Test func posts_messages_with_time() throws
    {
        let scenario = try LIST_SCENARIO()
        defer {
            scenario.close()
        }
        let now: Date = try #require(Calendar.current.date(from: DateComponents(year: 2026,
            month: 11, day: 9, hour: 15, minute: 42, second: 10)))
        let status = STATUS_BAR_VIEW_MODEL(list: scenario.list, now: { now })

        status.post("8번 항목을 직접 수정했습니다.")

        #expect(status.message == "8번 항목을 직접 수정했습니다.")
        #expect(status.time_text == "15:42")
    }

    @Test func toggles_filter_from_problem_counts() throws
    {
        let scenario = try STATUS_SCENARIO.create()
        defer {
            scenario.close()
        }
        let status = STATUS_BAR_VIEW_MODEL(list: scenario.list, now: { Date() })

        #expect(status.has_errors && status.has_conflicts && status.has_warnings)
        #expect(status.error_text == "오류 1")
        status.toggle_error()
        #expect(scenario.list.filter == .error)
        #expect(status.active_filter == .error)
        status.toggle_conflict()
        #expect(scenario.list.filter == .conflict)
        status.toggle_conflict()
        #expect(scenario.list.filter == .all)
    }

    @Test func shows_progress_after_delay() throws
    {
        let scenario = try LIST_SCENARIO()
        defer {
            scenario.close()
        }
        var now = Date(timeIntervalSince1970: 1_800_000_000)
        let status = STATUS_BAR_VIEW_MODEL(list: scenario.list, now: { now })
        let progress = PROGRESS()

        status.begin_progress(progress, label: "이름 바꾸는 중")
        progress.set(420, 1000)
        now += 0.2
        status.poll_progress()
        let early: Bool = status.progress_visible
        now += 0.11
        status.poll_progress()

        #expect(!early)
        #expect(status.progress_visible)
        #expect(status.progress_label == "이름 바꾸는 중")
        #expect(status.progress_text == "420 / 1,000")
        #expect(abs(status.progress_value - 0.42) < 1e-9)
        status.end_progress()
        #expect(!status.progress_visible)
        #expect(status.progress_text == "")
    }
}
