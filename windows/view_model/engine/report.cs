using System.Collections.Generic;

namespace limenamer;

public sealed record REPORT(
    IReadOnlyList<REPORT_ITEM> problems,
    uint total,
    uint succeeded,
    uint skipped,
    uint failed,
    bool needs_choice);
