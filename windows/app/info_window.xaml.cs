using System;
using System.IO;
using System.Text;
using System.Windows;

namespace limenamer;

public partial class INFO_WINDOW : Window
{
    public INFO_WINDOW()
    {
        InitializeComponent();
        string version = APP.VERSION_TOKEN[(APP.VERSION_TOKEN.IndexOf('=') + 1)..];
        version_text.Text = JOSA_FORMATTER.format(STRINGS.info_version, version);
        StringBuilder notices = new();
        notices.AppendLine("LimeNamer");
        notices.AppendLine(read_text("licenses/limenamer.txt"));
        notices.AppendLine("nlohmann/json 3.12.0");
        notices.AppendLine("https://github.com/nlohmann/json");
        notices.AppendLine();
        notices.Append(read_text("licenses/nlohmann_json.txt"));
        notices_text.Text = notices.ToString();
    }

    private void close_click(object sender, RoutedEventArgs args)
    {
        Close();
    }

    private static string read_text(string path)
    {
        Uri uri = new($"pack://application:,,,/{path}");
        using Stream stream = Application.GetResourceStream(uri).Stream;
        using StreamReader reader = new(stream, Encoding.UTF8);
        return reader.ReadToEnd();
    }
}
