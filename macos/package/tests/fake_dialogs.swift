import limenamer_view_model

@MainActor
final class FAKE_DIALOGS: DIALOG_SERVICE
{
    var answers: [Bool] = []
    private(set) var asked: [DIALOG_CONTENT] = []

    func ask(_ content: DIALOG_CONTENT) async -> Bool
    {
        asked.append(content)
        return answers.isEmpty || answers.removeFirst()
    }
}
