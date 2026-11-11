import limenamer_view_model

final class MEMORY_STORE: SETTINGS_STORE
{
    var values: [String: Any] = [:]

    func object(forKey key: String) -> Any?
    {
        return values[key]
    }

    func set(_ value: Any?, forKey key: String)
    {
        values[key] = value
    }
}
