using System.Threading;

namespace limenamer;

public sealed class PROGRESS
{
    private long value_;

    public (uint done, uint total) read()
    {
        long value = Interlocked.Read(ref value_);
        return ((uint)value, (uint)(value >> 32));
    }

    internal void set(uint done, uint total)
    {
        Interlocked.Exchange(ref value_, (long)((ulong)total << 32 | done));
    }
}
