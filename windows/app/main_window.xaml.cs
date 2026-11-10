using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Threading;
using Microsoft.Win32;

namespace limenamer;

public partial class MAIN_WINDOW : Window
{
    private const double PANEL_WIDTH = 360;

    private readonly MAIN_VIEW_MODEL main_;
    private readonly DispatcherTimer progress_timer_;
    private readonly List<TextBlock> labels_ = new();
    private double panel_width_ = PANEL_WIDTH;
    private bool compact_;

    public MAIN_WINDOW(ENGINE engine, SETTINGS settings)
    {
        InitializeComponent();
        main_ = new MAIN_VIEW_MODEL(engine, settings, new WPF_DIALOGS(this),
            () => DateTime.Now);
        DataContext = main_;
        list_view.attach(main_);
        panel_view.attach(main_);
        list_view.add_requested += () => add_files_click(this, new RoutedEventArgs());
        progress_timer_ = new DispatcherTimer { Interval = TimeSpan.FromMilliseconds(50) };
        progress_timer_.Tick += (_, _) => main_.status.poll_progress();
        main_.PropertyChanged += on_main_changed;
        Activated += (_, _) => main_.activated();
        main_.list.PropertyChanged += on_list_changed;
        Loaded += on_loaded;
        SizeChanged += (_, _) => update_panel_limit();
        toolbar.SizeChanged += (_, size) => update_toolbar(size.NewSize.Width);
    }

    private void on_loaded(object sender, RoutedEventArgs args)
    {
        collect_labels(toolbar);
        apply_panel();
        update_panel_limit();
        update_toolbar(toolbar.ActualWidth);
    }

    private void on_list_changed(object? sender, PropertyChangedEventArgs args)
    {
        if (args.PropertyName == nameof(ITEM_LIST_VIEW_MODEL.filter_label)) {
            Dispatcher.BeginInvoke(() => update_toolbar(toolbar.ActualWidth),
                DispatcherPriority.Loaded);
        }
    }

    private void on_main_changed(object? sender, PropertyChangedEventArgs args)
    {
        if (args.PropertyName == nameof(MAIN_VIEW_MODEL.is_running)) {
            if (main_.is_running) {
                progress_timer_.Start();
            } else {
                progress_timer_.Stop();
            }
        } else if (args.PropertyName == nameof(MAIN_VIEW_MODEL.panel_open)) {
            apply_panel();
        }
    }

    private void apply_panel()
    {
        if (main_.panel_open) {
            panel_column.MinWidth = 300;
            panel_column.Width = new GridLength(panel_width_);
            splitter_column.Width = new GridLength(10);
            panel.Visibility = Visibility.Visible;
            splitter.Visibility = Visibility.Visible;
        } else {
            if (panel_column.ActualWidth > 0) {
                panel_width_ = panel_column.ActualWidth;
            }
            panel_column.MinWidth = 0;
            panel_column.Width = new GridLength(0);
            splitter_column.Width = new GridLength(0);
            panel.Visibility = Visibility.Collapsed;
            splitter.Visibility = Visibility.Collapsed;
        }
    }

    private void update_panel_limit()
    {
        panel_column.MaxWidth = Math.Max(300, content.ActualWidth / 2);
    }

    private void update_toolbar(double available)
    {
        if (labels_.Count == 0) {
            return;
        }
        double full = content_width(toolbar_left) + content_width(toolbar_right);
        if (compact_) {
            foreach (TextBlock label in labels_) {
                full += label_width(label);
            }
        }
        bool compact = full > available + 0.5;
        if (compact == compact_) {
            return;
        }
        compact_ = compact;
        foreach (TextBlock label in labels_) {
            label.Visibility = compact ? Visibility.Collapsed : Visibility.Visible;
        }
    }

    private static double content_width(Panel panel)
    {
        double width = 0;
        foreach (UIElement child in panel.Children) {
            width += child.DesiredSize.Width;
        }
        return width;
    }

    private static double label_width(TextBlock label)
    {
        TextBlock probe = new() {
            Text = label.Text,
            FontFamily = label.FontFamily,
            FontSize = label.FontSize,
            FontWeight = label.FontWeight,
            FontStyle = label.FontStyle,
            FontStretch = label.FontStretch,
        };
        probe.Measure(new Size(double.PositiveInfinity, double.PositiveInfinity));
        return probe.DesiredSize.Width + label.Margin.Left + label.Margin.Right;
    }

    private void collect_labels(DependencyObject parent)
    {
        foreach (object child in LogicalTreeHelper.GetChildren(parent)) {
            if (child is TextBlock text && Equals(text.Tag, "label")) {
                labels_.Add(text);
            } else if (child is DependencyObject node) {
                collect_labels(node);
            }
        }
    }

    private void open_menu_click(object sender, RoutedEventArgs args)
    {
        FrameworkElement owner = (FrameworkElement)((FrameworkElement)sender).Parent;
        open_menu(owner.ContextMenu, owner);
    }

    private void open_own_menu_click(object sender, RoutedEventArgs args)
    {
        FrameworkElement owner = (FrameworkElement)sender;
        open_menu(owner.ContextMenu, owner);
    }

    private static void open_menu(ContextMenu? menu, FrameworkElement owner)
    {
        if (menu == null) {
            return;
        }
        menu.PlacementTarget = owner;
        menu.Placement = PlacementMode.Bottom;
        menu.IsOpen = true;
    }

    private void add_files_click(object sender, RoutedEventArgs args)
    {
        OpenFileDialog dialog = new() { Multiselect = true };
        if (dialog.ShowDialog(this) == true) {
            main_.add_paths(dialog.FileNames, ADD_MODE.as_items);
        }
    }

    private void add_folders_click(object sender, RoutedEventArgs args)
    {
        add_folders(ADD_MODE.as_items);
    }

    private void add_folder_contents_click(object sender, RoutedEventArgs args)
    {
        add_folders(ADD_MODE.folder_contents);
    }

    private void add_folders(ADD_MODE mode)
    {
        OpenFolderDialog dialog = new() { Multiselect = true };
        if (dialog.ShowDialog(this) == true) {
            main_.add_paths(dialog.FolderNames, mode);
        }
    }

    private void sort_menu_opened(object sender, RoutedEventArgs args)
    {
        sort_menu.Items.Clear();
        sort_menu.Items.Add(MENUS.title_item(STRINGS.sort_title));
        bool custom = main_.sort_key == SORT_KEY.custom;
        foreach (CHOICE<SORT_KEY> choice in MAIN_VIEW_MODEL.sort_choices) {
            sort_menu.Items.Add(MENUS.menu_item(choice.label, main_.sort_key == choice.value,
                () => main_.sort(choice.value, main_.sort_descending && !custom)));
        }
        if (custom) {
            MenuItem item = MENUS.menu_item(STRINGS.sort_custom, true, () => { });
            item.IsEnabled = false;
            sort_menu.Items.Add(item);
        }
        sort_menu.Items.Add(new Separator());
        SORT_KEY key = custom ? SORT_KEY.added : main_.sort_key;
        MenuItem ascending = MENUS.menu_item(STRINGS.sort_ascending,
            !custom && !main_.sort_descending, () => main_.sort(key, false));
        MenuItem descending = MENUS.menu_item(STRINGS.sort_descending,
            !custom && main_.sort_descending, () => main_.sort(key, true));
        ascending.IsEnabled = !custom;
        descending.IsEnabled = !custom;
        sort_menu.Items.Add(ascending);
        sort_menu.Items.Add(descending);
    }

    private void filter_menu_opened(object sender, RoutedEventArgs args)
    {
        filter_menu.Items.Clear();
        filter_menu.Items.Add(MENUS.title_item(STRINGS.filter_title));
        ITEM_LIST_VIEW_MODEL list = main_.list;
        (VIEW_FILTER filter, long count)[] filters = [
            (VIEW_FILTER.all, list.count_all),
            (VIEW_FILTER.changed, list.count_changed),
            (VIEW_FILTER.warning, list.count_warning),
            (VIEW_FILTER.error, list.count_error),
            (VIEW_FILTER.conflict, list.count_conflict),
        ];
        foreach ((VIEW_FILTER filter, long count) in filters) {
            string label = JOSA_FORMATTER.format(STRINGS.filter_item,
                ITEM_LIST_VIEW_MODEL.filter_name(filter), count);
            filter_menu.Items.Add(MENUS.menu_item(label, list.filter == filter,
                () => list.filter = filter));
        }
    }

    private void columns_menu_opened(object sender, RoutedEventArgs args)
    {
        MENUS.fill_columns(columns_menu, main_.settings);
    }

    private void theme_menu_opened(object sender, RoutedEventArgs args)
    {
        theme_menu.Items.Clear();
        theme_menu.Items.Add(MENUS.title_item(STRINGS.theme_title));
        foreach (CHOICE<THEME_MODE> choice in MAIN_VIEW_MODEL.theme_choices) {
            theme_menu.Items.Add(MENUS.menu_item(choice.label,
                main_.settings.theme_mode == choice.value, () => main_.set_theme(choice.value)));
        }
    }

    private void theme_click(object sender, RoutedEventArgs args)
    {
        main_.toggle_theme(THEME_SWITCHER.system_is_dark());
    }

    private void duplicate_click(object sender, RoutedEventArgs args)
    {
        duplicate_popup.IsOpen = true;
    }

    private void info_click(object sender, RoutedEventArgs args)
    {
        new INFO_WINDOW { Owner = this }.ShowDialog();
    }
}
