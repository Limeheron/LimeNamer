enum UTF8_INPUT
{
    static func is_valid(_ text: String) -> Bool
    {
        return !text.utf8.contains(0)
    }

    static func with_c_string<R>(_ text: String,
        _ body: (UnsafePointer<CChar>) -> R) throws(ENGINE_ERROR) -> R
    {
        guard is_valid(text) else {
            throw ENGINE_ERROR(result: .invalid_argument)
        }
        return text.withCString(body)
    }
}
