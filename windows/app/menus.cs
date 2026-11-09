using System;
using System.Windows.Controls;

namespace limenamer;

public static class MENUS
{
    public static MenuItem title_item(string text)
    {
        return new MenuItem {
            Header = text,
            IsEnabled = false,
            FontSize = 12,
        };
    }

    public static MenuItem menu_item(string text, bool selected, Action action)
    {
        MenuItem item = new() { Header = text, IsChecked = selected };
        item.Click += (_, _) => action();
        return item;
    }

    public static MenuItem check_item(string text, bool selected, Action<bool> action)
    {
        MenuItem item = new() { Header = text, IsCheckable = true, IsChecked = selected };
        item.Click += (_, _) => action(item.IsChecked);
        return item;
    }

    public static void fill_columns(ContextMenu menu, SETTINGS settings)
    {
        menu.Items.Clear();
        menu.Items.Add(title_item(STRINGS.columns_title));
        menu.Items.Add(check_item(STRINGS.column_index, settings.show_index,
            value => settings.show_index = value));
        menu.Items.Add(check_item(STRINGS.column_status, settings.show_status,
            value => settings.show_status = value));
        menu.Items.Add(check_item(STRINGS.column_folder, settings.show_folder,
            value => settings.show_folder = value));
        menu.Items.Add(check_item(STRINGS.column_modified, settings.show_modified,
            value => settings.show_modified = value));
        menu.Items.Add(check_item(STRINGS.column_created, settings.show_created,
            value => settings.show_created = value));
        menu.Items.Add(check_item(STRINGS.column_size, settings.show_size,
            value => settings.show_size = value));
    }
}
