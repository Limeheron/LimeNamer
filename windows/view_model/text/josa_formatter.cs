using System;
using System.Globalization;
using System.Text;

namespace limenamer;

internal sealed class JOSA_FORMATTER : IFormatProvider, ICustomFormatter
{
    private const int RIEUL = 8;
    private static readonly int[] DIGIT_FINALS = [21, 8, 0, 16, 0, 0, 1, 8, 8, 0];
    private static readonly JOSA_FORMATTER INSTANCE = new();

    internal static string format(string template, params object[] args)
    {
        return string.Format(INSTANCE, template, args);
    }

    internal static int final_consonant(string text)
    {
        int end = text.Length;
        while (end > 0 && is_closing(text[end - 1])) {
            end--;
        }
        if (end == 0) {
            return 0;
        }

        Rune.DecodeLastFromUtf16(text.AsSpan(0, end), out Rune last, out _);
        int code = last.Value;
        if (code >= 0xAC00 && code <= 0xD7A3) {
            return (code - 0xAC00) % 28;
        }
        if (code >= '0' && code <= '9') {
            return DIGIT_FINALS[code - '0'];
        }
        if (code == 'l' || code == 'L') {
            return RIEUL;
        }
        if (code == 'm' || code == 'n' || code == 'M' || code == 'N') {
            return 4;
        }
        return 0;
    }

    public object? GetFormat(Type? format_type)
    {
        return format_type == typeof(ICustomFormatter) ? this : null;
    }

    public string Format(string? format, object? arg, IFormatProvider? provider)
    {
        string[]? forms = format?.Split('/');
        if (forms == null || forms.Length != 2) {
            if (arg is IFormattable formattable) {
                return formattable.ToString(format, CultureInfo.CurrentCulture);
            }
            return arg?.ToString() ?? "";
        }

        string text = arg?.ToString() ?? "";
        int final = final_consonant(text);
        bool has_final = forms[0] == "으로" ? final != 0 && final != RIEUL : final != 0;
        return text + (has_final ? forms[0] : forms[1]);
    }

    private static bool is_closing(char value)
    {
        return value == '"' || value == '\'' || value == '}' || value == ']' || value == ')'
            || char.IsWhiteSpace(value);
    }
}
