namespace limenamer;

public sealed record NUMBER_SETTINGS
{
    public int start { get; init; } = 1;
    public int step { get; init; } = 1;
    public uint digits { get; init; } = 1;
}
