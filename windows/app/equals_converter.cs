using System;
using System.Globalization;
using System.Windows;
using System.Windows.Data;

namespace limenamer;

public sealed class EQUALS_CONVERTER : IValueConverter
{
    public object Convert(object? value, Type target_type, object? parameter, CultureInfo culture)
    {
        bool equal = Equals(value, parameter);
        if (target_type == typeof(Visibility)) {
            return equal ? Visibility.Visible : Visibility.Collapsed;
        }
        return equal;
    }

    public object ConvertBack(object? value, Type target_type, object? parameter,
        CultureInfo culture)
    {
        throw new NotSupportedException();
    }
}
