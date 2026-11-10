import Foundation

enum SUMMARY_CASES
{
    static func load() throws -> [SUMMARY_CASE]
    {
        let data = try Data(contentsOf: TEST_PATHS.TESTDATA_DIR.appendingPathComponent(
            "mod_summary_cases.json"))
        guard let items = try JSONSerialization.jsonObject(with: data) as? [[String: Any]] else {
            throw CocoaError(.fileReadCorruptFile)
        }
        var cases: [SUMMARY_CASE] = []
        for item in items {
            guard let name = item["name"] as? String, let mod = item["mod"],
                let expected = item["expected"] as? String else {
                throw CocoaError(.fileReadCorruptFile)
            }
            let mod_json = String(decoding: try JSONSerialization.data(withJSONObject: mod),
                as: UTF8.self)
            cases.append(SUMMARY_CASE(name: name, mod_json: mod_json,
                direct_index: (item["direct_index"] as? UInt32) ?? 0,
                target_count: (item["target_count"] as? UInt32) ?? 0, expected: expected))
        }
        return cases
    }
}
