namespace limenamer;

public sealed record SIZE_SETTINGS
{
    public SIZE_UNIT unit { get; init; } = SIZE_UNIT.auto;
}
