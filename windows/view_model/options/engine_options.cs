namespace limenamer;

public sealed record ENGINE_OPTIONS
{
    public DUPLICATE_OPTIONS duplicate { get; init; } = new();
}
