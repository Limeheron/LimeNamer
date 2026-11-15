using System;
using System.Globalization;
using System.Windows.Data;

namespace limenamer;

public sealed class SAME_CONVERTER : IMultiValueConverter
{
    public object Convert(object?[] values, Type target_type, object? parameter,
        CultureInfo culture)
    {
        return values.Length == 2 && Equals(values[0], values[1]);
    }

    public object?[] ConvertBack(object? value, Type[] target_types, object? parameter,
        CultureInfo culture)
    {
        throw new NotSupportedException();
    }
}
