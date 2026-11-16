import Observation

import limenamer_view_model

@MainActor
@Observable
final class SHEET_DIALOGS: DIALOG_SERVICE
{
    private(set) var current: DIALOG_CONTENT? = nil
    @ObservationIgnored private var waiting: CheckedContinuation<Bool, Never>? = nil

    func ask(_ content: DIALOG_CONTENT) async -> Bool
    {
        answer(false)
        return await withCheckedContinuation { continuation in
            waiting = continuation
            current = content
        }
    }

    func answer(_ primary: Bool)
    {
        let continuation: CheckedContinuation<Bool, Never>? = waiting
        waiting = nil
        current = nil
        continuation?.resume(returning: primary)
    }
}
