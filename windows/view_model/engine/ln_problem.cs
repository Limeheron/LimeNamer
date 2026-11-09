using System.Runtime.InteropServices;

namespace limenamer;

[StructLayout(LayoutKind.Sequential)]
internal struct LN_PROBLEM
{
    public int field;
    public int reason;
}
