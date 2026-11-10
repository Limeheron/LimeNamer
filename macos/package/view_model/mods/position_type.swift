public enum POSITION_TYPE: String, Codable, Sendable
{
    case start
    case end
    case after_nth
    case before_nth_from_end
    case before_text
    case after_text
    case all
    case first_n
    case last_n
    case from_nth
    case text
    case between
}
