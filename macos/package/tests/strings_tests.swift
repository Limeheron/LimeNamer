import Testing

import limenamer_view_model

struct STRINGS_TESTS
{
    @Test func matches_the_windows_strings() throws
    {
        let entries: [(key: String, value: String)] = try RESX_READER.read(TEST_PATHS.STRINGS_RESX)

        #expect(entries.count > 300)
        for entry in entries {
            #expect(STRINGS.text(entry.key) == entry.value, "\(entry.key)")
        }
    }

    @Test func reads_strings_through_accessors()
    {
        #expect(STRINGS.status_conflict == "충돌")
        #expect(STRINGS.summary_scope == " ({0}개)")
    }
}
