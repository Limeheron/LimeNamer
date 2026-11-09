using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace limenamer;

internal static unsafe class NATIVE_READER
{
    internal static string read_string(byte *text)
    {
        return Encoding.UTF8.GetString(MemoryMarshal.CreateReadOnlySpanFromNullTerminated(text));
    }

    internal static string take_string(byte *text)
    {
        try {
            return read_string(text);
        } finally {
            NATIVE_METHODS.ln_string_free(text);
        }
    }

    internal static PREVIEW take_preview(LN_PREVIEW *preview)
    {
        try {
            ROW[] rows = new ROW[checked((int)preview->count)];
            Dictionary<nint, string> folders = new();
            for (int index = 0; index < rows.Length; index++) {
                LN_ROW *row = preview->rows + index;
                if (!folders.TryGetValue((nint)row->folder, out string? folder)) {
                    folder = read_string(row->folder);
                    folders.Add((nint)row->folder, folder);
                }
                rows[index] = new ROW(row->item_id, folder, read_string(row->name),
                    read_string(row->new_name), row->size, row->modified_ms, row->created_ms,
                    row->prefix_len, row->suffix_len, (NAME_ISSUE)row->errors,
                    (NAME_ISSUE)row->warnings, (STATUS)row->status, (CONFLICT)row->conflict,
                    row->conflict_index, row->is_dir != 0, row->changed != 0, row->adjusted != 0,
                    row->direct != 0);
            }
            return new PREVIEW(rows, preview->changed, preview->runnable, preview->warning,
                preview->error, preview->conflict);
        } finally {
            NATIVE_METHODS.ln_preview_free(preview);
        }
    }

    internal static EXAMPLE take_example(LN_EXAMPLE *example)
    {
        try {
            return new EXAMPLE(example->item_id, example->index, read_string(example->before),
                read_string(example->after), example->prefix_len, example->suffix_len,
                (NAME_ISSUE)example->errors, (EXAMPLE_STATE)example->state);
        } finally {
            NATIVE_METHODS.ln_example_free(example);
        }
    }

    internal static REPORT take_report(LN_REPORT *report)
    {
        try {
            REPORT_ITEM[] problems = new REPORT_ITEM[checked((int)report->problem_count)];
            for (int index = 0; index < problems.Length; index++) {
                LN_REPORT_ITEM *item = report->problems + index;
                problems[index] = new REPORT_ITEM(item->item_id, read_string(item->from_path),
                    read_string(item->to_path), read_string(item->current_path),
                    (STAGE)item->stage, (FAILURE)item->failure, item->os_error);
            }
            return new REPORT(problems, report->total, report->succeeded, report->skipped,
                report->failed, report->needs_choice != 0);
        } finally {
            NATIVE_METHODS.ln_report_free(report);
        }
    }

    internal static PROBLEM[] take_problems(LN_PROBLEMS *problems)
    {
        try {
            PROBLEM[] items = new PROBLEM[checked((int)problems->count)];
            for (int index = 0; index < items.Length; index++) {
                LN_PROBLEM item = problems->items[index];
                items[index] = new PROBLEM((MOD_FIELD)item.field, (PROBLEM_REASON)item.reason);
            }
            return items;
        } finally {
            NATIVE_METHODS.ln_problems_free(problems);
        }
    }

    internal static INDEX_LIST take_index_list(LN_INDEX_LIST *list)
    {
        try {
            uint[] indices = new ReadOnlySpan<uint>(list->indices, checked((int)list->count))
                .ToArray();
            return new INDEX_LIST(indices, (INDEX_ERROR)list->error);
        } finally {
            NATIVE_METHODS.ln_index_list_free(list);
        }
    }
}
