namespace limenamer;

public sealed record REPORT_ITEM(
    ulong item_id,
    string from_path,
    string to_path,
    string current_path,
    STAGE stage,
    FAILURE failure,
    int os_error);
