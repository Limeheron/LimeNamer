namespace limenamer;

public readonly record struct ROW(
    ulong item_id,
    string folder,
    string name,
    string new_name,
    long size,
    long modified_ms,
    long created_ms,
    uint prefix_len,
    uint suffix_len,
    NAME_ISSUE errors,
    NAME_ISSUE warnings,
    STATUS status,
    CONFLICT conflict,
    uint conflict_index,
    bool is_dir,
    bool changed,
    bool adjusted,
    bool direct);
