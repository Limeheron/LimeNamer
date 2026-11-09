using System.Runtime.InteropServices;

namespace limenamer;

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct LN_REPORT_ITEM
{
    public ulong item_id;
    public byte *from_path;
    public byte *to_path;
    public byte *current_path;
    public int stage;
    public int failure;
    public int os_error;
}
