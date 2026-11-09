using System.Collections.Generic;

namespace limenamer;

public sealed record MOD
{
    public ulong? id { get; init; }
    public bool? enabled { get; init; }
    public MOD_KIND kind { get; init; }
    public PART part { get; init; } = PART.name;
    public POSITION position { get; init; } = new();
    public IReadOnlyList<VALUE_PIECE>? value { get; init; }
    public TOKEN_SETTINGS? tokens { get; init; }
    public FORMAT_SETTINGS? format { get; init; }
    public SCOPE scope { get; init; } = SCOPE.all;
    public IReadOnlyList<ulong>? targets { get; init; }
    public bool direct { get; init; }
}
