using System.Runtime.InteropServices;

namespace limenamer;

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct LN_EXAMPLE
{
    public ulong item_id;
    public uint index;
    public byte *before;
    public byte *after;
    public uint prefix_len;
    public uint suffix_len;
    public uint errors;
    public int state;
}
