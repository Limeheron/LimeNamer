using System.Runtime.InteropServices;

namespace limenamer;

[StructLayout(LayoutKind.Sequential)]
internal struct LN_ADD_RESULT
{
    public uint added;
    public uint skipped;
    public uint failed;
}
