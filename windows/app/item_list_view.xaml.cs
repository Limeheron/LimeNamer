using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Media3D;

namespace limenamer;

public partial class ITEM_LIST_VIEW : UserControl
{
    private const double SCROLL_EDGE = 16;

    private readonly ContextMenu header_menu_ = new();
    private readonly Dictionary<DataGridColumn, Style> header_styles_ = new();
    private readonly Dictionary<Style, Style> first_styles_ = new();
    private MAIN_VIEW_MODEL? main_;
    private bool syncing_;
    private bool allow_edit_;
    private ITEM_ROW? menu_row_;
    private IReadOnlyList<ulong>? drag_ids_;
    private (ulong target_id, bool after)? drop_slot_;
    private bool drop_valid_;

    public ITEM_LIST_VIEW()
    {
        InitializeComponent();
        grid.SelectionChanged += on_selection_changed;
        grid.BeginningEdit += on_beginning_edit;
        grid.CellEditEnding += on_cell_edit_ending;
        grid.PreviewMouseRightButtonDown += on_right_button_down;
        grid.ContextMenuOpening += on_context_menu_opening;
        grid.PreviewMouseLeftButtonDown += on_left_button_down;
        grid.PreviewMouseMove += on_mouse_move;
        grid.PreviewMouseLeftButtonUp += on_left_button_up;
        grid.LostMouseCapture += (_, _) => end_drag();
        grid.PreviewKeyDown += on_key_down;
        header_menu_.Opened += (_, _) => MENUS.fill_columns(header_menu_, main.settings);
        DragEnter += on_file_drag;
        DragOver += on_file_drag;
        DragLeave += on_file_leave;
        Drop += on_file_drop;
    }

    public event Action? add_requested;

    private MAIN_VIEW_MODEL main
    {
        get { return main_ ?? throw new InvalidOperationException("View is not attached"); }
    }

    public void attach(MAIN_VIEW_MODEL main)
    {
        main_ = main;
        main.list.PropertyChanged += on_list_changed;
        main.list.selection_requested += apply_selection;
        main.list.reveal_requested += reveal;
        main.settings.PropertyChanged += (_, _) => apply_columns();
        show_rows();
        apply_columns();
    }

    public void select_all()
    {
        grid.SelectAll();
    }

    public void edit_selected()
    {
        if (grid.SelectedItem is ITEM_ROW row && !is_editing()) {
            begin_direct_edit(row);
        }
    }

    private void on_list_changed(object? sender, PropertyChangedEventArgs args)
    {
        if (args.PropertyName == nameof(ITEM_LIST_VIEW_MODEL.rows)) {
            show_rows();
        } else if (args.PropertyName == nameof(ITEM_LIST_VIEW_MODEL.designating)) {
            apply_columns();
        }
    }

    private void show_rows()
    {
        syncing_ = true;
        try {
            grid.ItemsSource = main.list.rows;
        } finally {
            syncing_ = false;
        }
        apply_selection(main.list.selected_ids);
    }

    private void apply_selection(IReadOnlyCollection<ulong> ids)
    {
        syncing_ = true;
        try {
            HashSet<ulong> wanted = ids.ToHashSet();
            grid.select_items(main.list.rows.Where(row => wanted.Contains(row.item_id))
                .ToArray());
        } finally {
            syncing_ = false;
        }
    }

    private void reveal(ulong item_id)
    {
        ITEM_ROW? row = main.list.rows.FirstOrDefault(item => item.item_id == item_id);
        if (row != null) {
            grid.ScrollIntoView(row);
        }
    }

    private void apply_columns()
    {
        SETTINGS settings = main.settings;
        bool designating = main.list.designating;
        check_column.Visibility = visible(designating);
        index_column.Visibility = visible(settings.show_index || designating);
        status_column.Visibility = visible(settings.show_status);
        folder_column.Visibility = visible(settings.show_folder);
        modified_column.Visibility = visible(settings.show_modified);
        created_column.Visibility = visible(settings.show_created);
        size_column.Visibility = visible(settings.show_size);
        mark_first_header();
    }

    private void mark_first_header()
    {
        DataGridColumn? first = grid.Columns
            .Where(column => column.Visibility == Visibility.Visible)
            .OrderBy(column => column.DisplayIndex).FirstOrDefault();
        foreach (DataGridColumn column in grid.Columns) {
            if (!header_styles_.TryGetValue(column, out Style? style)) {
                style = column.HeaderStyle ?? grid.ColumnHeaderStyle;
                header_styles_[column] = style;
            }
            column.HeaderStyle = column == first ? first_style(style) : style;
        }
    }

    private Style first_style(Style style)
    {
        if (!first_styles_.TryGetValue(style, out Style? first)) {
            first = new Style(typeof(DataGridColumnHeader), style);
            first.Setters.Add(new Setter(TagProperty, "first"));
            first_styles_[style] = first;
        }
        return first;
    }

    private void on_selection_changed(object sender, SelectionChangedEventArgs args)
    {
        if (!syncing_) {
            main.list.set_selection(
                grid.SelectedItems.Cast<ITEM_ROW>().Select(row => row.item_id));
        }
    }

    private void on_key_down(object sender, KeyEventArgs args)
    {
        if (args.Key == Key.F2 && grid.SelectedItem is ITEM_ROW row && !is_editing()) {
            begin_direct_edit(row);
            args.Handled = true;
        }
    }

    private void new_name_mouse_down(object sender, MouseButtonEventArgs args)
    {
        if (args.ClickCount == 2 && ((FrameworkElement)sender).DataContext is ITEM_ROW row) {
            begin_direct_edit(row);
            args.Handled = true;
        }
    }

    private void begin_direct_edit(ITEM_ROW row)
    {
        if (!main.can_start_direct_edit()) {
            return;
        }
        grid.SelectedItem = row;
        grid.ScrollIntoView(row, new_column);
        grid.CurrentCell = new DataGridCellInfo(row, new_column);
        allow_edit_ = true;
        try {
            grid.BeginEdit();
        } finally {
            allow_edit_ = false;
        }
    }

    private void on_beginning_edit(object? sender, DataGridBeginningEditEventArgs args)
    {
        if (!allow_edit_) {
            args.Cancel = true;
        }
    }

    private void edit_box_loaded(object sender, RoutedEventArgs args)
    {
        TextBox box = (TextBox)sender;
        box.Focus();
        int dot = box.DataContext is ITEM_ROW { is_dir: false } ? box.Text.LastIndexOf('.') : -1;
        if (dot > 0) {
            box.Select(0, dot);
        } else {
            box.SelectAll();
        }
    }

    private void on_cell_edit_ending(object? sender, DataGridCellEditEndingEventArgs args)
    {
        if (args.EditAction != DataGridEditAction.Commit || args.Column != new_column
                || args.Row.Item is not ITEM_ROW row) {
            return;
        }
        TextBox? box = find_child<TextBox>(args.EditingElement);
        if (box == null || box.Text == row.new_name) {
            return;
        }
        string text = box.Text;
        Dispatcher.BeginInvoke(() => main.direct_edit(row.item_id, text));
    }

    private bool is_editing()
    {
        return grid.CurrentCell.Column == new_column
            && Keyboard.FocusedElement is TextBox box && is_inside(box, grid);
    }

    private void on_right_button_down(object sender, MouseButtonEventArgs args)
    {
        DependencyObject? source = args.OriginalSource as DependencyObject;
        if (find_parent<DataGridColumnHeader>(source) != null) {
            header_menu_.PlacementTarget = grid;
            header_menu_.IsOpen = true;
            args.Handled = true;
            return;
        }
        DataGridRow? row = find_parent<DataGridRow>(source);
        menu_row_ = row?.Item as ITEM_ROW;
        if (row != null && !row.IsSelected) {
            grid.SelectedItem = row.Item;
        }
    }

    private void on_context_menu_opening(object sender, ContextMenuEventArgs args)
    {
        if (menu_row_ == null || !main.can_edit) {
            args.Handled = true;
        }
    }

    private void row_direct_edit_click(object sender, RoutedEventArgs args)
    {
        if (menu_row_ != null) {
            begin_direct_edit(menu_row_);
        }
    }

    private void row_remove_click(object sender, RoutedEventArgs args)
    {
        main.remove_selected();
    }

    private void row_open_location_click(object sender, RoutedEventArgs args)
    {
        string? path = menu_row_ == null ? null : main.location_of(menu_row_.item_id);
        if (path != null) {
            Process.Start(new ProcessStartInfo("explorer.exe", $"/select,\"{path}\"") {
                UseShellExecute = true,
            });
        }
    }

    private void check_click(object sender, RoutedEventArgs args)
    {
        if (((FrameworkElement)sender).DataContext is ITEM_ROW row) {
            bool range = Keyboard.Modifiers.HasFlag(ModifierKeys.Shift);
            main.list.toggle_check(row.item_id, range);
        }
    }

    private void check_all_click(object sender, RoutedEventArgs args)
    {
        main.list.check_all(main.list.check_all_state != true);
    }

    private void empty_add_click(object sender, RoutedEventArgs args)
    {
        add_requested?.Invoke();
    }

    private void show_all_click(object sender, RoutedEventArgs args)
    {
        main.list.filter = VIEW_FILTER.all;
    }

    private void on_left_button_down(object sender, MouseButtonEventArgs args)
    {
        DependencyObject? source = args.OriginalSource as DependencyObject;
        if (find_parent<DataGridRow>(source) == null
                && find_parent<DataGridColumnHeader>(source) == null
                && find_parent<ScrollBar>(source) == null) {
            grid.UnselectAll();
        }
    }

    private void grip_mouse_down(object sender, MouseButtonEventArgs args)
    {
        if (((FrameworkElement)sender).DataContext is not ITEM_ROW row || !main.list.can_drag
                || !main.can_edit) {
            return;
        }
        drag_ids_ = main.list.drag_ids(row.item_id);
        drop_slot_ = null;
        drop_valid_ = false;
        drag_text.Text = drag_ids_.Count == 1
            ? row.name
            : JOSA_FORMATTER.format(STRINGS.drag_count, drag_ids_.Count);
        move_badge(args.GetPosition(grid));
        drop_line.Visibility = Visibility.Collapsed;
        drop_dot.Visibility = Visibility.Collapsed;
        drop_layer.Visibility = Visibility.Visible;
        grid.CaptureMouse();
        args.Handled = true;
    }

    private void move_badge(Point point)
    {
        Canvas.SetLeft(drag_badge, point.X + 14);
        Canvas.SetTop(drag_badge, point.Y + 14);
    }

    private void on_mouse_move(object sender, MouseEventArgs args)
    {
        if (drag_ids_ == null) {
            return;
        }
        Point point = args.GetPosition(grid);
        move_badge(point);
        scroll_near_edge(point);
        (ulong target_id, bool after, double y)? slot = slot_at(point);
        (ulong, bool)? key = slot == null ? null : (slot.Value.target_id, slot.Value.after);
        if (key != drop_slot_) {
            drop_slot_ = key;
            drop_valid_ = slot != null
                && main.list.can_drop(drag_ids_, slot.Value.target_id, slot.Value.after);
        }
        Visibility line = visible(slot != null && drop_valid_);
        drop_line.Visibility = line;
        drop_dot.Visibility = line;
        if (slot == null || !drop_valid_) {
            return;
        }
        double y = slot.Value.y;
        Canvas.SetLeft(drop_line, 8);
        Canvas.SetTop(drop_line, y - 1);
        drop_line.Width = Math.Max(0, grid.ActualWidth - 16);
        Canvas.SetLeft(drop_dot, 4);
        Canvas.SetTop(drop_dot, y - 4);
    }

    private void on_left_button_up(object sender, MouseButtonEventArgs args)
    {
        if (drag_ids_ != null) {
            IReadOnlyList<ulong> ids = drag_ids_;
            (ulong target_id, bool after)? slot = drop_slot_;
            bool valid = drop_valid_;
            end_drag();
            if (valid && slot != null) {
                main.move_rows(ids, slot.Value.target_id, slot.Value.after);
            }
            args.Handled = true;
            return;
        }
        DataGridRow? row = find_parent<DataGridRow>(args.OriginalSource as DependencyObject);
        if (row?.Item is ITEM_ROW item) {
            main.show_row_problems(item.item_id);
        }
    }

    private void end_drag()
    {
        if (drag_ids_ == null) {
            return;
        }
        drag_ids_ = null;
        drop_slot_ = null;
        drop_layer.Visibility = Visibility.Collapsed;
        if (grid.IsMouseCaptured) {
            grid.ReleaseMouseCapture();
        }
    }

    private (ulong target_id, bool after, double y)? slot_at(Point point)
    {
        IReadOnlyList<ITEM_ROW> rows = main.list.rows;
        if (rows.Count == 0) {
            return null;
        }
        DataGridRow? row = find_parent<DataGridRow>(grid.InputHitTest(point) as DependencyObject);
        if (row == null || row.Item is not ITEM_ROW item) {
            DataGridRow? last = grid.ItemContainerGenerator.ContainerFromItem(rows[^1])
                as DataGridRow;
            if (last == null || point.Y < last.TranslatePoint(new Point(0, 0), grid).Y) {
                return null;
            }
            double bottom = last.TranslatePoint(new Point(0, last.ActualHeight), grid).Y;
            return (rows[^1].item_id, true, bottom);
        }
        double top = row.TranslatePoint(new Point(0, 0), grid).Y;
        bool after = point.Y > top + row.ActualHeight / 2;
        double y = after ? top + row.ActualHeight : top;
        int index = rows.Count - 1;
        for (int position = 0; position < rows.Count; position++) {
            if (rows[position].item_id == item.item_id) {
                index = position;
                break;
            }
        }
        if (!after && index > 0) {
            return (rows[index - 1].item_id, true, y);
        }
        return (item.item_id, after, y);
    }

    private void scroll_near_edge(Point point)
    {
        ScrollViewer? scroller = find_child<ScrollViewer>(grid);
        if (scroller == null) {
            return;
        }
        if (point.Y < grid.ColumnHeaderHeight + SCROLL_EDGE) {
            scroller.LineUp();
        } else if (point.Y > grid.ActualHeight - SCROLL_EDGE) {
            scroller.LineDown();
        }
    }

    private void on_file_drag(object sender, DragEventArgs args)
    {
        bool files = main_ != null && main.can_edit
            && args.Data.GetDataPresent(DataFormats.FileDrop);
        args.Effects = files ? DragDropEffects.Copy : DragDropEffects.None;
        show_drop_zone(files);
        args.Handled = true;
    }

    private void on_file_leave(object sender, DragEventArgs args)
    {
        show_drop_zone(false);
    }

    private void show_drop_zone(bool shown)
    {
        drop_zone.Visibility = visible(shown);
        empty_layer.Visibility = visible(!shown);
    }

    private void on_file_drop(object sender, DragEventArgs args)
    {
        show_drop_zone(false);
        if (main.can_edit && args.Data.GetData(DataFormats.FileDrop) is string[] paths) {
            main.add_paths(paths, ADD_MODE.as_items);
        }
        args.Handled = true;
    }

    private static Visibility visible(bool value)
    {
        return value ? Visibility.Visible : Visibility.Collapsed;
    }

    private static bool is_inside(DependencyObject element, DependencyObject ancestor)
    {
        for (DependencyObject? node = element; node != null; node = parent_of(node)) {
            if (node == ancestor) {
                return true;
            }
        }
        return false;
    }

    private static T? find_parent<T>(DependencyObject? element) where T : DependencyObject
    {
        for (DependencyObject? node = element; node != null; node = parent_of(node)) {
            if (node is T found) {
                return found;
            }
        }
        return null;
    }

    private static DependencyObject? parent_of(DependencyObject node)
    {
        if (node is Visual || node is Visual3D) {
            return VisualTreeHelper.GetParent(node);
        }
        return LogicalTreeHelper.GetParent(node);
    }

    private static T? find_child<T>(DependencyObject? element) where T : DependencyObject
    {
        if (element == null) {
            return null;
        }
        for (int index = 0; index < VisualTreeHelper.GetChildrenCount(element); index++) {
            DependencyObject child = VisualTreeHelper.GetChild(element, index);
            if (child is T found) {
                return found;
            }
            T? nested = find_child<T>(child);
            if (nested != null) {
                return nested;
            }
        }
        return null;
    }
}
