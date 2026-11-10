import Testing

import limenamer_view_model

struct ENGINE_STATIC_TESTS
{
    @Test func reads_version()
    {
        let version: String = ENGINE.version()

        #expect(version.wholeMatch(of: /\d+\.\d+\.\d+/) != nil, "\(version)")
    }

    @Test func parses_index_lists() throws
    {
        let list: INDEX_LIST = try ENGINE.index_list_parse("3-5, 1", item_count: 10)
        let syntax: INDEX_LIST = try ENGINE.index_list_parse("1,,2", item_count: 10)
        let range: INDEX_LIST = try ENGINE.index_list_parse("11", item_count: 10)

        #expect(list == INDEX_LIST(indices: [1, 3, 4, 5], error: .none))
        #expect(syntax.error == .syntax)
        #expect(range.error == .out_of_range)
        #expect(throws: ENGINE_ERROR(result: .invalid_argument)) {
            try ENGINE.index_list_parse("1\u{0}", item_count: 10)
        }
    }

    @Test func formats_index_lists() throws
    {
        #expect(try ENGINE.index_list_format([9, 1, 2, 4, 5, 6, 5]) == "1, 2, 4-6, 9")
        #expect(try ENGINE.index_list_format([]) == "")
    }
}
