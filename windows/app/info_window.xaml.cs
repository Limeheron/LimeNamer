using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Documents;
using System.Windows.Navigation;

namespace limenamer;

public partial class INFO_WINDOW : Window
{
    private const string JSON_NAME = "nlohmann/json 3.12.0";
    private const string JSON_URL = "https://github.com/nlohmann/json";

    public INFO_WINDOW()
    {
        InitializeComponent();
        string version = APP.VERSION_TOKEN[(APP.VERSION_TOKEN.IndexOf('=') + 1)..];
        version_text.Text = JOSA_FORMATTER.format(STRINGS.info_version, version);
        add_notice("LimeNamer", null, "licenses/limenamer.txt");
        notices_text.Inlines.Add(new LineBreak());
        notices_text.Inlines.Add(new LineBreak());
        add_notice(JSON_NAME, JSON_URL, "licenses/nlohmann_json.txt");
    }

    private void close_click(object sender, RoutedEventArgs args)
    {
        Close();
    }

    private void link_navigate(object sender, RequestNavigateEventArgs args)
    {
        Process.Start(new ProcessStartInfo(args.Uri.AbsoluteUri) { UseShellExecute = true });
        args.Handled = true;
    }

    private void add_notice(string name, string? url, string path)
    {
        InlineCollection inlines = notices_text.Inlines;
        inlines.Add(new Bold(new Run(name)));
        inlines.Add(new LineBreak());
        if (url != null) {
            Hyperlink link = new(new Run(url)) { NavigateUri = new Uri(url) };
            link.RequestNavigate += link_navigate;
            inlines.Add(link);
            inlines.Add(new LineBreak());
        }
        inlines.Add(new Run(read_license(path)));
    }

    private static string read_license(string path)
    {
        Uri uri = new($"pack://application:,,,/{path}");
        using Stream stream = Application.GetResourceStream(uri).Stream;
        using StreamReader reader = new(stream, Encoding.UTF8);
        string text = reader.ReadToEnd().Replace("\r\n", "\n").Trim();
        return string.Join("\n\n", text.Split("\n\n").Select(part => part.Replace('\n', ' ')));
    }
}
