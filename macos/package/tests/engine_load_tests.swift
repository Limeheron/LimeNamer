import Testing

import limenamer_view_model

struct ENGINE_LOAD_TESTS
{
    @Test func engine_links_and_reports_version()
    {
        #expect(ENGINE.version() == "0.1.0")
    }
}
