import limenamer_engine

public final class ENGINE
{
    public static func version() -> String
    {
        return String(cString: ln_version())
    }
}
