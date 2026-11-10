public struct SIZE_SETTINGS: Codable, Equatable, Sendable
{
    public var unit: SIZE_UNIT

    public init(unit: SIZE_UNIT = .auto)
    {
        self.unit = unit
    }

    public init(from decoder: any Decoder) throws
    {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        unit = try container.decodeIfPresent(SIZE_UNIT.self, forKey: .unit) ?? .auto
    }
}
