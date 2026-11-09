namespace limenamer;

public sealed record TOKEN_SETTINGS
{
    public NUMBER_SETTINGS? number { get; init; }
    public DATE_SETTINGS? modified_date { get; init; }
    public DATE_SETTINGS? created_date { get; init; }
    public SIZE_SETTINGS? size { get; init; }
}
