using System.Runtime.InteropServices;

namespace limenamer;

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct LN_ROW
{
    public ulong item_id;
    public byte *folder;
    public byte *name;
    public byte *new_name;
    public long size;
    public long modified_ms;
    public long created_ms;
    public uint prefix_len;
    public uint suffix_len;
    public uint errors;
    public uint warnings;
    public int status;
    public int conflict;
    public uint conflict_index;
    public byte is_dir;
    public byte changed;
    public byte adjusted;
    public byte direct;
}
