namespace limenamer;

public sealed record DUPLICATE_OPTIONS
{
    public bool enabled { get; init; }
    public DUPLICATE_FORMAT format { get; init; } = DUPLICATE_FORMAT.paren;
    public string? custom_text { get; init; }
    public DUPLICATE_MODE mode { get; init; } = DUPLICATE_MODE.keep_first;
}
