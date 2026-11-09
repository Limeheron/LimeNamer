using System;
using System.Globalization;
using System.Windows;
using System.Windows.Data;

namespace limenamer;

public sealed class HAS_TEXT_CONVERTER : IValueConverter
{
    public object Convert(object? value, Type target_type, object? parameter, CultureInfo culture)
    {
        bool has_text = value is string text && text.Length > 0;
        if (target_type == typeof(Visibility)) {
            return has_text ? Visibility.Visible : Visibility.Collapsed;
        }
        return has_text;
    }

    public object ConvertBack(object? value, Type target_type, object? parameter,
        CultureInfo culture)
    {
        throw new NotSupportedException();
    }
}
