namespace limenamer;

public sealed record POSITION
{
    public POSITION_TYPE type { get; init; }
    public uint? n { get; init; }
    public uint? count { get; init; }
    public string? text { get; init; }
    public bool? case_sensitive { get; init; }
    public OCCURRENCE? occurrence { get; init; }
    public uint? occurrence_n { get; init; }
    public bool? from_beginning { get; init; }
    public string? start_text { get; init; }
    public bool? to_end { get; init; }
    public string? end_text { get; init; }
    public bool? include_delimiters { get; init; }
}
