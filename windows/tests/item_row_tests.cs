using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace limenamer;

[TestClass]
public sealed class ITEM_ROW_TESTS
{
    [TestMethod]
    public void splits_highlight_by_code_points()
    {
        Assert.AreEqual(("a", "\U0001F600b", "c"), ITEM_ROW.split("a\U0001F600bc", 1, 1));
        Assert.AreEqual(("", "new_", "a.txt"), ITEM_ROW.split("new_a.txt", 0, 5));
        Assert.AreEqual(("abc", "", ""), ITEM_ROW.split("abc", 3, 0));
    }

    [TestMethod]
    public void formats_sizes()
    {
        Assert.AreEqual("3B", ITEM_ROW.size_of(3));
        Assert.AreEqual("2KB", ITEM_ROW.size_of(1536));
        Assert.AreEqual("2.5MB", ITEM_ROW.size_of(2621440));
        Assert.AreEqual("3.0GB", ITEM_ROW.size_of(3221225472));
    }
}
