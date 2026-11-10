import Foundation

public enum MOD_JSON
{
    public static func parse(_ json: String) throws -> MOD
    {
        return try JSONDecoder().decode(MOD.self, from: Data(json.utf8))
    }

    public static func parse_list(_ json: String) throws -> [MOD]
    {
        return try JSONDecoder().decode([MOD].self, from: Data(json.utf8))
    }

    public static func write(_ mod: MOD) -> String
    {
        return encode(mod)
    }

    public static func parse_options(_ json: String) throws -> ENGINE_OPTIONS
    {
        return try JSONDecoder().decode(ENGINE_OPTIONS.self, from: Data(json.utf8))
    }

    public static func write_options(_ options: ENGINE_OPTIONS) -> String
    {
        return encode(options)
    }

    private static func encode<T: Encodable>(_ value: T) -> String
    {
        let encoder = JSONEncoder()
        encoder.outputFormatting = [.withoutEscapingSlashes]
        guard let data = try? encoder.encode(value) else {
            return ""
        }
        return String(decoding: data, as: UTF8.self)
    }
}
