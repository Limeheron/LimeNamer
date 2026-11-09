using System.Runtime.InteropServices;

namespace limenamer;

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct LN_INDEX_LIST
{
    public uint *indices;
    public nuint count;
    public int error;
}
