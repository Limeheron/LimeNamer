public struct NUMBER_SETTINGS: Codable, Equatable, Sendable
{
    public var start: Int32
    public var step: Int32
    public var digits: UInt32

    public init(start: Int32 = 1, step: Int32 = 1, digits: UInt32 = 1)
    {
        self.start = start
        self.step = step
        self.digits = digits
    }

    public init(from decoder: any Decoder) throws
    {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        start = try container.decodeIfPresent(Int32.self, forKey: .start) ?? 1
        step = try container.decodeIfPresent(Int32.self, forKey: .step) ?? 1
        digits = try container.decodeIfPresent(UInt32.self, forKey: .digits) ?? 1
    }
}
