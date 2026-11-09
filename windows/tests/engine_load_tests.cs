using System;
using System.IO;
using System.Runtime.InteropServices;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace limenamer;

[TestClass]
public sealed class ENGINE_LOAD_TESTS
{
    [TestMethod]
    public void engine_dll_loads_and_exports_functions()
    {
        string path = Path.Combine(AppContext.BaseDirectory, "limenamer_engine.dll");

        Assert.IsTrue(NativeLibrary.TryLoad(path, out IntPtr handle), path);
        try {
            Assert.IsTrue(NativeLibrary.TryGetExport(handle, "ln_version", out IntPtr address));
            Assert.AreNotEqual(IntPtr.Zero, address);
        } finally {
            NativeLibrary.Free(handle);
        }
    }
}
