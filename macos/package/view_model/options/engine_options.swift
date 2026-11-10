public struct ENGINE_OPTIONS: Codable, Equatable, Sendable
{
    public var duplicate: DUPLICATE_OPTIONS

    public init(duplicate: DUPLICATE_OPTIONS = DUPLICATE_OPTIONS())
    {
        self.duplicate = duplicate
    }

    public init(from decoder: any Decoder) throws
    {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        duplicate = try container.decodeIfPresent(DUPLICATE_OPTIONS.self, forKey: .duplicate)
            ?? DUPLICATE_OPTIONS()
    }
}
