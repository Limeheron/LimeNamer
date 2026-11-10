import Testing

@testable import limenamer_view_model

@MainActor
struct ITEM_ROW_TESTS
{
    @Test func splits_highlight_by_code_points()
    {
        #expect(ITEM_ROW.split("a\u{1F600}bc", prefix_len: 1, suffix_len: 1)
            == ("a", "\u{1F600}b", "c"))
        #expect(ITEM_ROW.split("new_a.txt", prefix_len: 0, suffix_len: 5) == ("", "new_", "a.txt"))
        #expect(ITEM_ROW.split("abc", prefix_len: 3, suffix_len: 0) == ("abc", "", ""))
        #expect(ITEM_ROW.split("e\u{301}x", prefix_len: 1, suffix_len: 1) == ("e", "\u{301}", "x"))
    }

    @Test func formats_sizes()
    {
        #expect(ITEM_ROW.size_of(3) == "3B")
        #expect(ITEM_ROW.size_of(1536) == "2KB")
        #expect(ITEM_ROW.size_of(2560) == "2KB")
        #expect(ITEM_ROW.size_of(2621440) == "2.5MB")
        #expect(ITEM_ROW.size_of(3221225472) == "3.0GB")
    }
}
