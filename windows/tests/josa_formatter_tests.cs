using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace limenamer;

[TestClass]
public sealed class JOSA_FORMATTER_TESTS
{
    [TestMethod]
    [DataRow("사진", "을/를", "사진을")]
    [DataRow("나무", "을/를", "나무를")]
    [DataRow("그림", "으로/로", "그림으로")]
    [DataRow("서울", "으로/로", "서울로")]
    [DataRow("바다", "으로/로", "바다로")]
    [DataRow("책", "과/와", "책과")]
    [DataRow("\"IMG_\"", "을/를", "\"IMG_\"를")]
    [DataRow("\"3\"", "으로/로", "\"3\"으로")]
    [DataRow("\"1\"", "으로/로", "\"1\"로")]
    [DataRow("html", "을/를", "html을")]
    [DataRow("{번호}", "과/와", "{번호}와")]
    [DataRow("", "을/를", "를")]
    [DataRow("\U0001F600", "을/를", "\U0001F600를")]
    public void attaches_particles(string text, string forms, string expected)
    {
        Assert.AreEqual(expected, JOSA_FORMATTER.format("{0:" + forms + "}", text));
    }

    [TestMethod]
    public void keeps_other_formats()
    {
        Assert.AreEqual("a 007", JOSA_FORMATTER.format("{0} {1:D3}", "a", 7));
    }
}
