using System.Runtime.CompilerServices;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace limenamer;

[TestClass]
public sealed class NATIVE_LAYOUT_TESTS
{
    [TestMethod]
    public void matches_c_struct_sizes()
    {
        Assert.AreEqual(88, Unsafe.SizeOf<LN_ROW>());
        Assert.AreEqual(40, Unsafe.SizeOf<LN_PREVIEW>());
        Assert.AreEqual(48, Unsafe.SizeOf<LN_EXAMPLE>());
        Assert.AreEqual(48, Unsafe.SizeOf<LN_REPORT_ITEM>());
        Assert.AreEqual(40, Unsafe.SizeOf<LN_REPORT>());
        Assert.AreEqual(8, Unsafe.SizeOf<LN_PROBLEM>());
        Assert.AreEqual(16, Unsafe.SizeOf<LN_PROBLEMS>());
        Assert.AreEqual(24, Unsafe.SizeOf<LN_INDEX_LIST>());
        Assert.AreEqual(12, Unsafe.SizeOf<LN_ADD_RESULT>());
    }
}
