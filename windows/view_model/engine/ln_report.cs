using System.Runtime.InteropServices;

namespace limenamer;

[StructLayout(LayoutKind.Sequential)]
internal unsafe struct LN_REPORT
{
    public LN_REPORT_ITEM *problems;
    public nuint problem_count;
    public uint total;
    public uint succeeded;
    public uint skipped;
    public uint failed;
    public byte needs_choice;
}
