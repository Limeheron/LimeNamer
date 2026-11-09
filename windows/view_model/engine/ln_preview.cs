using System.Runtime.InteropServices;

namespace limenamer;

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct LN_PREVIEW
{
    public LN_ROW *rows;
    public nuint count;
    public uint changed;
    public uint runnable;
    public uint warning;
    public uint error;
    public uint conflict;
}
