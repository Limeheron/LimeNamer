namespace limenamer;

public enum FAILURE
{
    none = 0,
    in_use = 1,
    access_denied = 2,
    not_found = 3,
    already_exists = 4,
    replaced = 5,
    path_too_long = 6,
    depends_on_failed = 7,
    other = 8
}
