public struct VALUE_PIECE: Codable, Equatable, Sendable
{
    public var text: String?
    public var token: TOKEN_KIND?

    public init(text: String? = nil, token: TOKEN_KIND? = nil)
    {
        self.text = text
        self.token = token
    }
}
