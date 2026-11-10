public struct FORMAT_SETTINGS: Codable, Equatable, Sendable
{
    public var type: FORMAT_TYPE
    public var digits: UInt32?

    public init(type: FORMAT_TYPE = .upper, digits: UInt32? = nil)
    {
        self.type = type
        self.digits = digits
    }

    public init(from decoder: any Decoder) throws
    {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        type = try container.decodeIfPresent(FORMAT_TYPE.self, forKey: .type) ?? .upper
        digits = try container.decodeIfPresent(UInt32.self, forKey: .digits)
    }
}
