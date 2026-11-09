using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Text;

namespace limenamer;

public sealed class ITEM_ROW : OBSERVABLE
{
    private bool is_checked_;
    private bool is_dimmed_;

    public ITEM_ROW(ROW row, uint index)
    {
        item_id = row.item_id;
        this.index = index;
        folder = row.folder;
        name = row.name;
        full_path = Path.Combine(row.folder, row.name);
        new_name = row.new_name;
        (new_name_head, new_name_mark, new_name_tail) = split(row.new_name, row.prefix_len,
            row.suffix_len);
        status = row.status;
        status_text = MESSAGES.status_name(row.status);
        changed = row.changed;
        direct = row.direct;
        is_dir = row.is_dir;
        problems = MESSAGES.row_messages(row);
        problem_text = string.Join("\n", problems);
        modified_text = date_text(row.modified_ms);
        created_text = date_text(row.created_ms);
        size_text = row.is_dir ? STRINGS.size_none : size_of(row.size);
    }

    public ulong item_id { get; }
    public uint index { get; }
    public string folder { get; }
    public string name { get; }
    public string full_path { get; }
    public string new_name { get; }
    public string new_name_head { get; }
    public string new_name_mark { get; }
    public string new_name_tail { get; }
    public STATUS status { get; }
    public string status_text { get; }
    public bool changed { get; }
    public bool direct { get; }
    public bool is_dir { get; }
    public IReadOnlyList<string> problems { get; }
    public string problem_text { get; }
    public string modified_text { get; }
    public string created_text { get; }
    public string size_text { get; }

    public bool is_checked
    {
        get { return is_checked_; }
        internal set { assign(ref is_checked_, value); }
    }

    public bool is_dimmed
    {
        get { return is_dimmed_; }
        internal set { assign(ref is_dimmed_, value); }
    }

    internal static (string head, string mark, string tail) split(string text, uint prefix_len,
        uint suffix_len)
    {
        int head_end = utf16_offset(text, 0, prefix_len, 1);
        int tail_start = Math.Max(head_end, utf16_offset(text, text.Length, suffix_len, -1));
        return (text[..head_end], text[head_end..tail_start], text[tail_start..]);
    }

    internal static string size_of(long bytes)
    {
        const double KB = 1024;
        if (bytes < KB) {
            return bytes.ToString(CultureInfo.InvariantCulture) + "B";
        }
        if (bytes < KB * KB) {
            return Math.Round(bytes / KB).ToString(CultureInfo.InvariantCulture) + "KB";
        }
        if (bytes < KB * KB * KB) {
            return (bytes / (KB * KB)).ToString("0.0", CultureInfo.InvariantCulture) + "MB";
        }
        return (bytes / (KB * KB * KB)).ToString("0.0", CultureInfo.InvariantCulture) + "GB";
    }

    private static int utf16_offset(string text, int start, uint code_points, int direction)
    {
        int offset = start;
        for (uint count = 0; count < code_points; count++) {
            if (direction > 0) {
                if (offset >= text.Length) {
                    break;
                }
                Rune.DecodeFromUtf16(text.AsSpan(offset), out _, out int used);
                offset += used;
            } else {
                if (offset <= 0) {
                    break;
                }
                Rune.DecodeLastFromUtf16(text.AsSpan(0, offset), out _, out int used);
                offset -= used;
            }
        }
        return offset;
    }

    private static string date_text(long milliseconds)
    {
        if (milliseconds <= 0) {
            return "";
        }
        return DateTimeOffset.FromUnixTimeMilliseconds(milliseconds).ToLocalTime()
            .ToString("yyyy-MM-dd HH:mm", CultureInfo.InvariantCulture);
    }
}
