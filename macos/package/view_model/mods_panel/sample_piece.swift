public struct SAMPLE_PIECE: Equatable, Sendable
{
    public let text: String
    public let style: SAMPLE_STYLE

    public init(text: String, style: SAMPLE_STYLE)
    {
        self.text = text
        self.style = style
    }
}
