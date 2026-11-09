namespace limenamer;

public sealed record VALUE_PIECE
{
    public string? text { get; init; }
    public TOKEN_KIND? token { get; init; }
}
