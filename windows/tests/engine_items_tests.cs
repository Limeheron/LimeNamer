using System;
using System.IO;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace limenamer;

[TestClass]
public sealed class ENGINE_ITEMS_TESTS
{
    [TestMethod]
    public void adds_items_and_reads_preview()
    {
        using TEMP_FOLDER temp = new();
        string a = temp.file("a.txt", "abc");
        string b = temp.file("b.txt");
        string photos = temp.folder("photos");
        using ENGINE engine = new();

        ADD_RESULT result = engine.items_add([a, b, photos], ADD_MODE.as_items);
        PREVIEW preview = engine.preview();

        Assert.AreEqual(new ADD_RESULT(3, 0, 0), result);
        Assert.AreEqual(3, preview.rows.Count);
        ROW first = preview.rows[0];
        Assert.AreEqual(temp.path, first.folder);
        Assert.AreEqual("a.txt", first.name);
        Assert.AreEqual("a.txt", first.new_name);
        Assert.AreEqual(3L, first.size);
        Assert.IsTrue(first.modified_ms > 0);
        Assert.AreEqual(STATUS.unchanged, first.status);
        Assert.IsFalse(first.changed);
        Assert.AreSame(first.folder, preview.rows[1].folder);
        Assert.IsTrue(preview.rows[2].is_dir);
        Assert.AreEqual(0u, preview.changed);
    }

    [TestMethod]
    public void counts_invalid_paths_as_failed()
    {
        using TEMP_FOLDER temp = new();
        string a = temp.file("a.txt");
        string broken = Path.Combine(temp.path, "\uD800.txt");
        using ENGINE engine = new();

        ADD_RESULT first = engine.items_add([a, broken], ADD_MODE.as_items);
        ADD_RESULT second = engine.items_add([a], ADD_MODE.as_items);

        Assert.AreEqual(new ADD_RESULT(1, 0, 1), first);
        Assert.AreEqual(new ADD_RESULT(0, 1, 0), second);
    }

    [TestMethod]
    public void adds_folder_contents()
    {
        using TEMP_FOLDER temp = new();
        temp.file("a.txt");
        temp.file("b.txt");
        using ENGINE engine = new();

        ADD_RESULT result = engine.items_add([temp.path], ADD_MODE.folder_contents);

        Assert.AreEqual(new ADD_RESULT(2, 0, 0), result);
    }

    [TestMethod]
    public void sorts_and_moves_items()
    {
        using TEMP_FOLDER temp = new();
        using ENGINE engine = new();
        engine.items_add([temp.file("a.txt"), temp.file("b.txt")], ADD_MODE.as_items);

        engine.items_sort(SORT_KEY.name_natural, true);
        PREVIEW sorted = engine.preview();
        ulong b_id = sorted.rows[0].item_id;
        ulong a_id = sorted.rows[1].item_id;

        Assert.AreEqual("b.txt", sorted.rows[0].name);
        Assert.AreEqual((SORT_KEY.name_natural, true), engine.items_sort_state());
        Assert.IsTrue(engine.items_move([a_id], b_id, false, true));
        Assert.AreEqual("b.txt", engine.preview().rows[0].name);
        Assert.IsTrue(engine.items_move([a_id], b_id, false, false));
        Assert.AreEqual("a.txt", engine.preview().rows[0].name);
        Assert.AreEqual(SORT_KEY.custom, engine.items_sort_state().key);
    }

    [TestMethod]
    public void removes_and_clears_items()
    {
        using TEMP_FOLDER temp = new();
        using ENGINE engine = new();
        engine.items_add([temp.file("a.txt"), temp.file("b.txt")], ADD_MODE.as_items);
        ulong first = engine.preview().rows[0].item_id;

        engine.items_remove([first]);
        ENGINE_EXCEPTION missing = Assert.ThrowsExactly<ENGINE_EXCEPTION>(
            () => engine.items_remove([first]));
        int remaining = engine.preview().rows.Count;
        engine.items_clear();

        Assert.AreEqual(RESULT.not_found, missing.result);
        Assert.AreEqual(1, remaining);
        Assert.AreEqual(0, engine.preview().rows.Count);
    }

    [TestMethod]
    public void refreshes_folders()
    {
        using TEMP_FOLDER temp = new();
        using ENGINE engine = new();
        engine.items_add([temp.file("a.txt")], ADD_MODE.as_items);
        engine.mod_add(LIST_SCENARIO.INSERT_NEW);
        temp.file("new_a.txt");

        STATUS before = engine.preview().rows[0].status;
        engine.folders_refresh();

        Assert.AreEqual(STATUS.ok, before);
        Assert.AreEqual(STATUS.conflict, engine.preview().rows[0].status);
    }

    [TestMethod]
    public void rejects_calls_after_dispose()
    {
        ENGINE engine = new();

        engine.Dispose();
        engine.Dispose();

        Assert.ThrowsExactly<ObjectDisposedException>(() => engine.items_clear());
    }
}
