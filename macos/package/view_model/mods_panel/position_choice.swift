public struct POSITION_CHOICE: Equatable, Sendable
{
    public let type: POSITION_TYPE
    public let label: String
    public let description: String
    public let before: [SAMPLE_PIECE]
    public let after: [SAMPLE_PIECE]
    public let tile: [SAMPLE_PIECE]

    public init(type: POSITION_TYPE, label: String, description: String, before: [SAMPLE_PIECE],
        after: [SAMPLE_PIECE], tile: [SAMPLE_PIECE])
    {
        self.type = type
        self.label = label
        self.description = description
        self.before = before
        self.after = after
        self.tile = tile
    }

    public var shows_after: Bool
    {
        return !after.isEmpty
    }
}
