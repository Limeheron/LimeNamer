public enum OCCURRENCE: String, Codable, Sendable
{
    case first
    case last
    case nth
    case nth_from_end
    case all
}
