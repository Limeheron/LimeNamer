using System.Windows;
using System.Windows.Threading;

namespace limenamer;

public partial class APP : Application
{
    public const string VERSION_TOKEN = "LIMENAMER_VERSION=0.1.0";

    private ENGINE? engine_;
    private THEME_SWITCHER? theme_;
    private bool reporting_;

    protected override void OnStartup(StartupEventArgs args)
    {
        base.OnStartup(args);
        DispatcherUnhandledException += on_unhandled;
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

    private void on_unhandled(object sender, DispatcherUnhandledExceptionEventArgs args)
    {
        if (MainWindow is not MAIN_WINDOW window || !window.IsLoaded) {
            return;
        }
        args.Handled = true;
        if (reporting_) {
            return;
        }
        reporting_ = true;
        try {
            DIALOG_CONTENT content = new(DIALOG_KIND.report, STRINGS.dialog_unexpected_title,
                STRINGS.dialog_unexpected_message, [], args.Exception.Message, STRINGS.dialog_ok,
                "");
            new DIALOG_WINDOW(content) { Owner = window }.ShowDialog();
        } finally {
            reporting_ = false;
        }
    }
}
