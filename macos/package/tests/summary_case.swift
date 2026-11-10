import Testing


struct SUMMARY_CASE: CustomTestStringConvertible, Sendable
{
    let name: String
    let mod_json: String
    let direct_index: UInt32
    let target_count: UInt32
    let expected: String

    var testDescription: String
    {
        return name
    }
}
