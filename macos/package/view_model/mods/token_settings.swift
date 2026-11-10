public struct TOKEN_SETTINGS: Codable, Equatable, Sendable
{
    public var number: NUMBER_SETTINGS?
    public var modified_date: DATE_SETTINGS?
    public var created_date: DATE_SETTINGS?
    public var size: SIZE_SETTINGS?

    public init(number: NUMBER_SETTINGS? = nil, modified_date: DATE_SETTINGS? = nil,
        created_date: DATE_SETTINGS? = nil, size: SIZE_SETTINGS? = nil)
    {
        self.number = number
        self.modified_date = modified_date
        self.created_date = created_date
        self.size = size
    }
}
