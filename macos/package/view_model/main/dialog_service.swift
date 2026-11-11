@MainActor
public protocol DIALOG_SERVICE
{
    func ask(_ content: DIALOG_CONTENT) async -> Bool
}
