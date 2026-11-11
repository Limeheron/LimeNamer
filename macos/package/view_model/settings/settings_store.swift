import Foundation

public protocol SETTINGS_STORE
{
    func object(forKey key: String) -> Any?
    func set(_ value: Any?, forKey key: String)
}

extension UserDefaults: SETTINGS_STORE
{
}
