namespace limenamer;

public sealed record EXAMPLE(
    ulong item_id,
    uint index,
    string before,
    string after,
    uint prefix_len,
    uint suffix_len,
    NAME_ISSUE errors,
    EXAMPLE_STATE state);
