using System;

namespace limenamer;

public sealed class ENGINE_EXCEPTION : Exception
{
    public ENGINE_EXCEPTION(RESULT result)
        : base($"Engine call failed: {result}")
    {
        this.result = result;
    }

    public RESULT result { get; }
}
