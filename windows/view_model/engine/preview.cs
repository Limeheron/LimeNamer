using System.Collections.Generic;

namespace limenamer;

public sealed record PREVIEW(
    IReadOnlyList<ROW> rows,
    uint changed,
    uint runnable,
    uint warning,
    uint error,
    uint conflict);
