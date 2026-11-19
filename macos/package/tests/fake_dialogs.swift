import limenamer_view_model

@MainActor
final class FAKE_DIALOGS: DIALOG_SERVICE
{
    var answers: [Bool] = []
    var on_ask: ((DIALOG_CONTENT) -> Void)?
    private(set) var asked: [DIALOG_CONTENT] = []

    func ask(_ content: DIALOG_CONTENT) async -> Bool
    {
        asked.append(content)
        on_ask?(content)
        return answers.isEmpty || answers.removeFirst()
    }
}
