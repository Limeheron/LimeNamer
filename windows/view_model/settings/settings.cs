using System;
using System.IO;
using System.Runtime.CompilerServices;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace limenamer;

public sealed class SETTINGS : OBSERVABLE
{
    private static readonly JsonSerializerOptions JSON = new()
    {
        WriteIndented = true,
        Converters = { new JsonStringEnumConverter(null, false) },
    };

    private readonly string path_;
    private SETTINGS_FILE file_;

    private SETTINGS(string path, SETTINGS_FILE file)
    {
        path_ = path;
        file_ = file;
    }

    public bool show_index
    {
        get { return file_.show_index; }
        set { update(file_ with { show_index = value }); }
    }

    public bool show_status
    {
        get { return file_.show_status; }
        set { update(file_ with { show_status = value }); }
    }

    public bool show_folder
    {
        get { return file_.show_folder; }
        set { update(file_ with { show_folder = value }); }
    }

    public bool show_modified
    {
        get { return file_.show_modified; }
        set { update(file_ with { show_modified = value }); }
    }

    public bool show_created
    {
        get { return file_.show_created; }
        set { update(file_ with { show_created = value }); }
    }

    public bool show_size
    {
        get { return file_.show_size; }
        set { update(file_ with { show_size = value }); }
    }

    public THEME_MODE theme_mode
    {
        get { return file_.theme_mode; }
        set { update(file_ with { theme_mode = value }); }
    }

    public bool example_collapsed
    {
        get { return file_.example_collapsed; }
        set { update(file_ with { example_collapsed = value }); }
    }

    public bool panel_open
    {
        get { return file_.panel_open; }
        set { update(file_ with { panel_open = value }); }
    }

    public static string default_path()
    {
        string folder = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
        return Path.Combine(folder, "LimeNamer", "settings.json");
    }

    public static SETTINGS load(string path)
    {
        SETTINGS_FILE file = new();
        try {
            if (File.Exists(path)) {
                file = JsonSerializer.Deserialize<SETTINGS_FILE>(File.ReadAllText(path), JSON)
                    ?? new SETTINGS_FILE();
            }
        } catch (Exception error) when (error is IOException or UnauthorizedAccessException
                or JsonException) {
            file = new SETTINGS_FILE();
        }
        return new SETTINGS(path, file);
    }

    private void update(SETTINGS_FILE file, [CallerMemberName] string? name = null)
    {
        if (file == file_) {
            return;
        }
        file_ = file;
        notify(name);
        save();
    }

    private void save()
    {
        try {
            Directory.CreateDirectory(Path.GetDirectoryName(path_)!);
            File.WriteAllText(path_, JsonSerializer.Serialize(file_, JSON));
        } catch (Exception error) when (error is IOException or UnauthorizedAccessException) {
            return;
        }
    }
}
