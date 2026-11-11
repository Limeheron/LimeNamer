public struct DIALOG_LINE: Equatable, Sendable
{
    public let icon: DIALOG_ICON
    public let index: UInt32
    public let text: String
    public let reason: String

    public init(icon: DIALOG_ICON, index: UInt32, text: String, reason: String)
    {
        self.icon = icon
        self.index = index
        self.text = text
        self.reason = reason
    }
}
