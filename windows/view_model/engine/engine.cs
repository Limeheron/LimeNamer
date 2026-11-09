using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace limenamer;

public sealed unsafe class ENGINE : IDisposable
{
    private readonly SESSION_HANDLE handle_;

    private delegate int RUN_CALL(delegate* unmanaged[Cdecl]<nint, uint, uint, void> progress,
        nint user, LN_REPORT **report);

    public ENGINE()
    {
        int result = NATIVE_METHODS.ln_session_create(out SESSION_HANDLE handle);
        if (result != (int)RESULT.ok) {
            handle.Dispose();
            throw new ENGINE_EXCEPTION((RESULT)result);
        }
        handle_ = handle;
    }

    public void Dispose()
    {
        handle_.Dispose();
    }

    public static string version()
    {
        return NATIVE_READER.read_string(NATIVE_METHODS.ln_version());
    }

    public ADD_RESULT items_add(IReadOnlyList<string> paths, ADD_MODE mode)
    {
        List<byte[]> encoded = new(paths.Count);
        uint invalid = 0;
        int size = 0;
        foreach (string path in paths) {
            byte[]? bytes = UTF8_INPUT.try_encode(path);
            if (bytes == null) {
                invalid++;
            } else {
                encoded.Add(bytes);
                size += bytes.Length;
            }
        }

        byte[] buffer = new byte[size];
        nint[] pointers = new nint[encoded.Count];
        LN_ADD_RESULT result = default;
        fixed (byte *start = buffer) {
            int offset = 0;
            for (int index = 0; index < encoded.Count; index++) {
                encoded[index].CopyTo(buffer, offset);
                pointers[index] = (nint)(start + offset);
                offset += encoded[index].Length;
            }
            fixed (nint *list = pointers) {
                check(NATIVE_METHODS.ln_items_add(handle_, (byte **)list,
                    (nuint)pointers.Length, (int)mode, &result));
            }
        }
        return new ADD_RESULT(result.added, result.skipped, result.failed + invalid);
    }

    public void items_remove(ReadOnlySpan<ulong> ids)
    {
        fixed (ulong *list = ids) {
            check(NATIVE_METHODS.ln_items_remove(handle_, list, (nuint)ids.Length));
        }
    }

    public void items_clear()
    {
        check(NATIVE_METHODS.ln_items_clear(handle_));
    }

    public bool items_move(ReadOnlySpan<ulong> ids, ulong target_id, bool after, bool dry_run)
    {
        byte changed = 0;
        fixed (ulong *list = ids) {
            check(NATIVE_METHODS.ln_items_move(handle_, list, (nuint)ids.Length, target_id,
                to_byte(after), to_byte(dry_run), &changed));
        }
        return changed != 0;
    }

    public void items_sort(SORT_KEY key, bool descending)
    {
        check(NATIVE_METHODS.ln_items_sort(handle_, (int)key, to_byte(descending)));
    }

    public (SORT_KEY key, bool descending) items_sort_state()
    {
        int key = 0;
        byte descending = 0;
        check(NATIVE_METHODS.ln_items_sort_state(handle_, &key, &descending));
        return ((SORT_KEY)key, descending != 0);
    }

    public void folders_refresh()
    {
        check(NATIVE_METHODS.ln_folders_refresh(handle_));
    }

    public string mods_get()
    {
        byte *json = null;
        check(NATIVE_METHODS.ln_mods_get(handle_, &json));
        return NATIVE_READER.take_string(json);
    }

    public ulong mod_add(string json)
    {
        ulong id = 0;
        fixed (byte *text = UTF8_INPUT.encode(json)) {
            check(NATIVE_METHODS.ln_mod_add(handle_, text, &id));
        }
        return id;
    }

    public void mod_update(ulong id, string json)
    {
        fixed (byte *text = UTF8_INPUT.encode(json)) {
            check(NATIVE_METHODS.ln_mod_update(handle_, id, text));
        }
    }

    public void mod_remove(ulong id)
    {
        check(NATIVE_METHODS.ln_mod_remove(handle_, id));
    }

    public bool mod_move(ulong id, ulong target_id, bool after, bool dry_run)
    {
        byte changed = 0;
        check(NATIVE_METHODS.ln_mod_move(handle_, id, target_id, to_byte(after),
            to_byte(dry_run), &changed));
        return changed != 0;
    }

    public void mod_set_enabled(ulong id, bool enabled)
    {
        check(NATIVE_METHODS.ln_mod_set_enabled(handle_, id, to_byte(enabled)));
    }

    public void mods_set_all_enabled(bool enabled)
    {
        check(NATIVE_METHODS.ln_mods_set_all_enabled(handle_, to_byte(enabled)));
    }

    public void mod_direct_edit(ulong item_id, string new_name)
    {
        fixed (byte *text = UTF8_INPUT.encode(new_name)) {
            check(NATIVE_METHODS.ln_mod_direct_edit(handle_, item_id, text));
        }
    }

    public IReadOnlyList<PROBLEM> mod_check(string json)
    {
        LN_PROBLEMS *problems = null;
        fixed (byte *text = UTF8_INPUT.encode(json)) {
            check(NATIVE_METHODS.ln_mod_check(handle_, text, &problems));
        }
        return NATIVE_READER.take_problems(problems);
    }

    public EXAMPLE mod_example(string json, ulong before_mod_id)
    {
        LN_EXAMPLE *example = null;
        fixed (byte *text = UTF8_INPUT.encode(json)) {
            check(NATIVE_METHODS.ln_mod_example(handle_, text, before_mod_id, &example));
        }
        return NATIVE_READER.take_example(example);
    }

    public string options_get()
    {
        byte *json = null;
        check(NATIVE_METHODS.ln_options_get(handle_, &json));
        return NATIVE_READER.take_string(json);
    }

    public void options_set(string json)
    {
        fixed (byte *text = UTF8_INPUT.encode(json)) {
            check(NATIVE_METHODS.ln_options_set(handle_, text));
        }
    }

    public PREVIEW preview()
    {
        LN_PREVIEW *preview = null;
        check(NATIVE_METHODS.ln_preview(handle_, &preview));
        return NATIVE_READER.take_preview(preview);
    }

    public void undo()
    {
        check(NATIVE_METHODS.ln_undo(handle_));
    }

    public void redo()
    {
        check(NATIVE_METHODS.ln_redo(handle_));
    }

    public (bool can_undo, bool can_redo) history_state()
    {
        byte can_undo = 0;
        byte can_redo = 0;
        check(NATIVE_METHODS.ln_history_state(handle_, &can_undo, &can_redo));
        return (can_undo != 0, can_redo != 0);
    }

    public REPORT rename(PROGRESS? progress)
    {
        return run(progress,
            (callback, user, report) => NATIVE_METHODS.ln_rename(handle_, callback, user, report));
    }

    public REPORT rename_resolve(RESOLVE_CHOICE choice, PROGRESS? progress)
    {
        return run(progress, (callback, user, report) =>
            NATIVE_METHODS.ln_rename_resolve(handle_, (int)choice, callback, user, report));
    }

    public REPORT rename_undo_check()
    {
        LN_REPORT *report = null;
        check(NATIVE_METHODS.ln_rename_undo_check(handle_, &report));
        return NATIVE_READER.take_report(report);
    }

    public REPORT rename_undo(PROGRESS? progress)
    {
        return run(progress, (callback, user, report) =>
            NATIVE_METHODS.ln_rename_undo(handle_, callback, user, report));
    }

    public (bool available, uint count) rename_undo_state()
    {
        byte available = 0;
        uint count = 0;
        check(NATIVE_METHODS.ln_rename_undo_state(handle_, &available, &count));
        return (available != 0, count);
    }

    public static INDEX_LIST index_list_parse(string text, uint item_count)
    {
        LN_INDEX_LIST *list = null;
        fixed (byte *input = UTF8_INPUT.encode(text)) {
            check(NATIVE_METHODS.ln_index_list_parse(input, item_count, &list));
        }
        return NATIVE_READER.take_index_list(list);
    }

    public static string index_list_format(ReadOnlySpan<uint> indices)
    {
        byte *text = null;
        fixed (uint *list = indices) {
            check(NATIVE_METHODS.ln_index_list_format(list, (nuint)indices.Length, &text));
        }
        return NATIVE_READER.take_string(text);
    }

    private static REPORT run(PROGRESS? progress, RUN_CALL call)
    {
        GCHandle target = progress == null ? default : GCHandle.Alloc(progress);
        try {
            delegate* unmanaged[Cdecl]<nint, uint, uint, void> callback = null;
            if (progress != null) {
                callback = &on_progress;
            }
            LN_REPORT *report = null;
            check(call(callback, GCHandle.ToIntPtr(target), &report));
            return NATIVE_READER.take_report(report);
        } finally {
            if (target.IsAllocated) {
                target.Free();
            }
        }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static void on_progress(nint user, uint done, uint total)
    {
        ((PROGRESS)GCHandle.FromIntPtr(user).Target!).set(done, total);
    }

    private static void check(int result)
    {
        if (result != (int)RESULT.ok) {
            throw new ENGINE_EXCEPTION((RESULT)result);
        }
    }

    private static byte to_byte(bool value)
    {
        return value ? (byte)1 : (byte)0;
    }
}
