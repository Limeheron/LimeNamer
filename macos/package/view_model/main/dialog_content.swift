public struct DIALOG_CONTENT: Equatable, Sendable
{
    public let kind: DIALOG_KIND
    public let title: String
    public let message: String
    public let lines: [DIALOG_LINE]
    public let note: String
    public let primary: String
    public let secondary: String

    public init(kind: DIALOG_KIND, title: String, message: String, lines: [DIALOG_LINE],
        note: String, primary: String, secondary: String)
    {
        self.kind = kind
        self.title = title
        self.message = message
        self.lines = lines
        self.note = note
        self.primary = primary
        self.secondary = secondary
    }
}
