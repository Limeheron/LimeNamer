import Testing

import limenamer_view_model

struct JOSA_FORMATTER_TESTS
{
    @Test(arguments: [
        ("사진", "을/를", "사진을"),
        ("나무", "을/를", "나무를"),
        ("그림", "으로/로", "그림으로"),
        ("서울", "으로/로", "서울로"),
        ("바다", "으로/로", "바다로"),
        ("책", "과/와", "책과"),
        ("\"IMG_\"", "을/를", "\"IMG_\"를"),
        ("\"3\"", "으로/로", "\"3\"으로"),
        ("\"1\"", "으로/로", "\"1\"로"),
        ("html", "을/를", "html을"),
        ("{번호}", "과/와", "{번호}와"),
        ("", "을/를", "를"),
        ("\u{1F600}", "을/를", "\u{1F600}를"),
    ])
    func attaches_particles(text: String, forms: String, expected: String)
    {
        #expect(JOSA_FORMATTER.format("{0:" + forms + "}", text) == expected)
    }

    @Test func keeps_plain_arguments_and_braces()
    {
        #expect(JOSA_FORMATTER.format("{0} {1}개", "a", UInt32(7)) == "a 7개")
        #expect(JOSA_FORMATTER.format("{{0}} {0}", "a") == "{0} a")
    }
}
