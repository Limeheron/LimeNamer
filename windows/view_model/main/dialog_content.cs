using System.Collections.Generic;

namespace limenamer;

public sealed record DIALOG_CONTENT(string title, string message, IReadOnlyList<DIALOG_LINE> lines,
    string note, string primary, string secondary);
