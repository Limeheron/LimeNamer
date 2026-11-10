using System;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;

namespace limenamer;

public partial class DIALOG_WINDOW : Window
{
    private const int GWL_STYLE = -16;
    private const long WS_SYSMENU = 0x00080000;

    private readonly DIALOG_KIND kind_;
    private bool answered_;

    public DIALOG_WINDOW(DIALOG_CONTENT content)
    {
        InitializeComponent();
        kind_ = content.kind;
        title_text.Text = content.title;
        message_text.Text = content.message;
        lines_view.ItemsSource = content.lines;
        lines_box.Visibility = content.lines.Count > 0 ? Visibility.Visible : Visibility.Collapsed;
        note_text.Text = content.note;
        note_text.Visibility = content.note.Length > 0 ? Visibility.Visible : Visibility.Collapsed;
        primary_button.Content = content.primary;
        secondary_button.Content = content.secondary;
        if (content.secondary.Length == 0) {
            secondary_button.Visibility = Visibility.Collapsed;
            buttons.Columns = 1;
            buttons.HorizontalAlignment = HorizontalAlignment.Right;
            primary_button.MinWidth = 120;
            primary_button.Margin = new Thickness(0);
        }
        if (content.kind != DIALOG_KIND.choice) {
            primary_button.SetResourceReference(StyleProperty, "accent_button");
            primary_button.IsDefault = true;
        }
        if (content.kind == DIALOG_KIND.confirm) {
            secondary_button.IsCancel = true;
        } else if (content.kind == DIALOG_KIND.report) {
            primary_button.IsCancel = true;
        }
        SourceInitialized += on_source_initialized;
        Closing += on_closing;
    }

    public bool chose_primary { get; private set; }

    private void primary_click(object sender, RoutedEventArgs args)
    {
        answer(true);
    }

    private void secondary_click(object sender, RoutedEventArgs args)
    {
        answer(false);
    }

    private void answer(bool primary)
    {
        chose_primary = primary;
        answered_ = true;
        Close();
    }

    private void on_closing(object? sender, CancelEventArgs args)
    {
        if (kind_ == DIALOG_KIND.choice && !answered_) {
            args.Cancel = true;
        }
    }

    private void on_source_initialized(object? sender, EventArgs args)
    {
        if (kind_ != DIALOG_KIND.choice) {
            return;
        }
        IntPtr handle = new WindowInteropHelper(this).Handle;
        long style = (long)get_window_long(handle, GWL_STYLE);
        set_window_long(handle, GWL_STYLE, (IntPtr)(style & ~WS_SYSMENU));
    }

    [DllImport("user32.dll", EntryPoint = "GetWindowLongPtrW")]
    private static extern IntPtr get_window_long(IntPtr handle, int index);

    [DllImport("user32.dll", EntryPoint = "SetWindowLongPtrW")]
    private static extern IntPtr set_window_long(IntPtr handle, int index, IntPtr value);
}
