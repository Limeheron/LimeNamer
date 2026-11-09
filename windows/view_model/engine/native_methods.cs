using System.Runtime.InteropServices;

namespace limenamer;

internal static unsafe partial class NATIVE_METHODS
{
    private const string LIBRARY = "limenamer_engine";

    [LibraryImport(LIBRARY)]
    internal static partial int ln_session_create(out SESSION_HANDLE session);

    [LibraryImport(LIBRARY)]
    internal static partial void ln_session_destroy(nint session);

    [LibraryImport(LIBRARY)]
    internal static partial byte *ln_version();

    [LibraryImport(LIBRARY)]
    internal static partial int ln_items_add(SESSION_HANDLE session, byte **paths, nuint count,
        int mode, LN_ADD_RESULT *result);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_items_remove(SESSION_HANDLE session, ulong *ids, nuint count);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_items_clear(SESSION_HANDLE session);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_items_move(SESSION_HANDLE session, ulong *ids, nuint count,
        ulong target_id, byte after, byte dry_run, byte *changed);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_items_sort(SESSION_HANDLE session, int key, byte descending);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_items_sort_state(SESSION_HANDLE session, int *key,
        byte *descending);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_folders_refresh(SESSION_HANDLE session);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_mods_get(SESSION_HANDLE session, byte **json);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_mod_add(SESSION_HANDLE session, byte *json, ulong *id);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_mod_update(SESSION_HANDLE session, ulong id, byte *json);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_mod_remove(SESSION_HANDLE session, ulong id);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_mod_move(SESSION_HANDLE session, ulong id, ulong target_id,
        byte after, byte dry_run, byte *changed);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_mod_set_enabled(SESSION_HANDLE session, ulong id, byte enabled);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_mods_set_all_enabled(SESSION_HANDLE session, byte enabled);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_mod_direct_edit(SESSION_HANDLE session, ulong item_id,
        byte *new_name);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_mod_check(SESSION_HANDLE session, byte *json,
        LN_PROBLEMS **problems);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_mod_example(SESSION_HANDLE session, byte *json,
        ulong before_mod_id, LN_EXAMPLE **example);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_options_get(SESSION_HANDLE session, byte **json);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_options_set(SESSION_HANDLE session, byte *json);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_preview(SESSION_HANDLE session, LN_PREVIEW **preview);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_undo(SESSION_HANDLE session);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_redo(SESSION_HANDLE session);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_history_state(SESSION_HANDLE session, byte *can_undo,
        byte *can_redo);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_rename(SESSION_HANDLE session,
        delegate* unmanaged[Cdecl]<nint, uint, uint, void> progress, nint user,
        LN_REPORT **report);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_rename_resolve(SESSION_HANDLE session, int choice,
        delegate* unmanaged[Cdecl]<nint, uint, uint, void> progress, nint user,
        LN_REPORT **report);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_rename_undo_check(SESSION_HANDLE session, LN_REPORT **report);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_rename_undo(SESSION_HANDLE session,
        delegate* unmanaged[Cdecl]<nint, uint, uint, void> progress, nint user,
        LN_REPORT **report);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_rename_undo_state(SESSION_HANDLE session, byte *available,
        uint *count);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_index_list_parse(byte *text, uint item_count,
        LN_INDEX_LIST **list);

    [LibraryImport(LIBRARY)]
    internal static partial int ln_index_list_format(uint *indices, nuint count, byte **text);

    [LibraryImport(LIBRARY)]
    internal static partial void ln_string_free(byte *text);

    [LibraryImport(LIBRARY)]
    internal static partial void ln_problems_free(LN_PROBLEMS *problems);

    [LibraryImport(LIBRARY)]
    internal static partial void ln_example_free(LN_EXAMPLE *example);

    [LibraryImport(LIBRARY)]
    internal static partial void ln_preview_free(LN_PREVIEW *preview);

    [LibraryImport(LIBRARY)]
    internal static partial void ln_report_free(LN_REPORT *report);

    [LibraryImport(LIBRARY)]
    internal static partial void ln_index_list_free(LN_INDEX_LIST *list);
}
