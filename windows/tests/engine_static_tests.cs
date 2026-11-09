using System.Text.RegularExpressions;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace limenamer;

[TestClass]
public sealed class ENGINE_STATIC_TESTS
{
    [TestMethod]
    public void reads_version()
    {
        string version = ENGINE.version();

        Assert.IsTrue(Regex.IsMatch(version, @"^\d+\.\d+\.\d+$"), version);
    }

    [TestMethod]
    public void parses_index_lists()
    {
        INDEX_LIST list = ENGINE.index_list_parse("3-5, 1", 10);
        INDEX_LIST syntax = ENGINE.index_list_parse("1,,2", 10);
        INDEX_LIST range = ENGINE.index_list_parse("11", 10);
        ENGINE_EXCEPTION broken = Assert.ThrowsExactly<ENGINE_EXCEPTION>(
            () => ENGINE.index_list_parse("\uD800", 10));

        CollectionAssert.AreEqual(new uint[] { 1, 3, 4, 5 }, (uint[])list.indices);
        Assert.AreEqual(INDEX_ERROR.none, list.error);
        Assert.AreEqual(INDEX_ERROR.syntax, syntax.error);
        Assert.AreEqual(INDEX_ERROR.out_of_range, range.error);
        Assert.AreEqual(RESULT.invalid_argument, broken.result);
    }

    [TestMethod]
    public void formats_index_lists()
    {
        Assert.AreEqual("1, 2, 4-6, 9", ENGINE.index_list_format([9, 1, 2, 4, 5, 6, 5]));
        Assert.AreEqual("", ENGINE.index_list_format([]));
    }
}
