using System.Collections.Generic;

namespace limenamer;

public sealed record POSITION_CHOICE(POSITION_TYPE type, string label, string description,
    IReadOnlyList<SAMPLE_PIECE> before, IReadOnlyList<SAMPLE_PIECE> after,
    IReadOnlyList<SAMPLE_PIECE> tile)
{
    public bool shows_after
    {
        get { return after.Count > 0; }
    }
}
