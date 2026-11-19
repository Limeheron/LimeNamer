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

    [TestMethod]
    public void shows_no_size_for_unknown_sizes()
    {
        ROW row = new(1, "docs", "Notes.rtfd", "Notes.rtfd", -1, 0, 0, 10, 0, NAME_ISSUE.none,
            NAME_ISSUE.none, STATUS.unchanged, CONFLICT.none, 0, false, false, false, false);
        Assert.AreEqual("—", new ITEM_ROW(row, 1).size_text);
    }
}
