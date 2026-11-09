namespace limenamer;

public sealed record FORMAT_SETTINGS
{
    public FORMAT_TYPE type { get; init; }
    public uint? digits { get; init; }
}
