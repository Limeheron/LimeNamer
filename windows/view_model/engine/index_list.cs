using System.Collections.Generic;

namespace limenamer;

public sealed record INDEX_LIST(IReadOnlyList<uint> indices, INDEX_ERROR error);
