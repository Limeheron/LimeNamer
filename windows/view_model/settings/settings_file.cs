namespace limenamer;

public sealed record SETTINGS_FILE
{
    public bool show_index { get; init; } = true;
    public bool show_status { get; init; } = true;
    public bool show_folder { get; init; }
    public bool show_modified { get; init; } = true;
    public bool show_created { get; init; }
    public bool show_size { get; init; }
    public THEME_MODE theme_mode { get; init; } = THEME_MODE.system;
    public bool example_collapsed { get; init; }
    public bool panel_open { get; init; } = true;
}
