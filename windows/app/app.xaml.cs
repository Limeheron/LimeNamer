using System.Windows;

namespace limenamer;

public partial class APP : Application
{
    public const string VERSION_TOKEN = "LIMENAMER_VERSION=0.1.0";

    private ENGINE? engine_;
    private THEME_SWITCHER? theme_;

    protected override void OnStartup(StartupEventArgs args)
    {
        base.OnStartup(args);
        engine_ = new ENGINE();
        SETTINGS settings = SETTINGS.load(SETTINGS.default_path());
        theme_ = new THEME_SWITCHER(this, settings);
        theme_.apply();
        MAIN_WINDOW window = new(engine_, settings);
        MainWindow = window;
        window.Show();
    }

    protected override void OnSessionEnding(SessionEndingCancelEventArgs args)
    {
        base.OnSessionEnding(args);
        if (MainWindow is MAIN_WINDOW window && !window.allow_close()) {
            args.Cancel = true;
        }
    }

    protected override void OnExit(ExitEventArgs args)
    {
        theme_?.Dispose();
        engine_?.Dispose();
        base.OnExit(args);
    }
}
