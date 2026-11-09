using System.Runtime.InteropServices;

namespace limenamer;

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct LN_PROBLEMS
{
    public LN_PROBLEM *items;
    public nuint count;
}
