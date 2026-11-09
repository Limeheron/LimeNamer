using System.Collections.Generic;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace limenamer;

[TestClass]
public sealed class ENGINE_MODS_TESTS
{
    private const string INSERT_NEW =
        """{"kind":"insert","position":{"type":"start"},"value":[{"text":"new_"}]}""";
    private const string INSERT_OLD =
        """{"kind":"insert","position":{"type":"start"},"value":[{"text":"old_"}]}""";

    [TestMethod]
    public void applies_mods_to_preview()
    {
        using TEMP_FOLDER temp = new();
        using ENGINE engine = new();
        engine.items_add([temp.file("a.txt")], ADD_MODE.as_items);

        ulong id = engine.mod_add(INSERT_NEW);
        PREVIEW preview = engine.preview();
        ROW row = preview.rows[0];

        Assert.AreEqual("new_a.txt", row.new_name);
        Assert.AreEqual(STATUS.ok, row.status);
        Assert.IsTrue(row.changed);
        Assert.AreEqual(0u, row.prefix_len);
        Assert.AreEqual(5u, row.suffix_len);
        Assert.AreEqual(1u, preview.changed);
        Assert.AreEqual(1u, preview.runnable);
        string mods = engine.mods_get();
        Assert.IsTrue(mods.Contains($"\"id\":{id}"), mods);
    }

    [TestMethod]
    public void updates_toggles_and_removes_mods()
    {
        using TEMP_FOLDER temp = new();
        using ENGINE engine = new();
        engine.items_add([temp.file("a.txt")], ADD_MODE.as_items);
        ulong first = engine.mod_add(INSERT_NEW);
        ulong second = engine.mod_add(INSERT_OLD);

        string both = engine.preview().rows[0].new_name;
        bool moved = engine.mod_move(second, first, false, false);
        string reordered = engine.preview().rows[0].new_name;
        engine.mod_update(first, INSERT_OLD);
        string updated = engine.preview().rows[0].new_name;
        engine.mods_set_all_enabled(false);
        string disabled = engine.preview().rows[0].new_name;
        engine.mod_set_enabled(first, true);
        string enabled = engine.preview().rows[0].new_name;
        engine.mod_remove(first);

        Assert.AreEqual("old_new_a.txt", both);
        Assert.IsTrue(moved);
        Assert.AreEqual("new_old_a.txt", reordered);
        Assert.AreEqual("old_old_a.txt", updated);
        Assert.AreEqual("a.txt", disabled);
        Assert.AreEqual("old_a.txt", enabled);
        Assert.AreEqual("a.txt", engine.preview().rows[0].new_name);
    }

    [TestMethod]
    public void reports_check_problems()
    {
        using ENGINE engine = new();

        IReadOnlyList<PROBLEM> problems = engine.mod_check(
            """{"kind":"replace","position":{"type":"text","text":""},"value":[{"text":"x"}]}""");

        Assert.Contains(new PROBLEM(MOD_FIELD.find_text, PROBLEM_REASON.required), problems);
    }

    [TestMethod]
    public void finds_examples()
    {
        using TEMP_FOLDER temp = new();
        using ENGINE engine = new();
        engine.items_add([temp.file("a.txt")], ADD_MODE.as_items);
        ulong item_id = engine.preview().rows[0].item_id;

        EXAMPLE example = engine.mod_example(INSERT_NEW, 0);

        Assert.AreEqual(item_id, example.item_id);
        Assert.AreEqual(1u, example.index);
        Assert.AreEqual("a.txt", example.before);
        Assert.AreEqual("new_a.txt", example.after);
        Assert.AreEqual(0u, example.prefix_len);
        Assert.AreEqual(5u, example.suffix_len);
        Assert.AreEqual(EXAMPLE_STATE.changed, example.state);
    }

    [TestMethod]
    public void edits_names_directly()
    {
        using TEMP_FOLDER temp = new();
        using ENGINE engine = new();
        engine.items_add([temp.file("a.txt")], ADD_MODE.as_items);
        ulong item_id = engine.preview().rows[0].item_id;

        engine.mod_direct_edit(item_id, "b.txt");
        ROW row = engine.preview().rows[0];
        ENGINE_EXCEPTION broken = Assert.ThrowsExactly<ENGINE_EXCEPTION>(
            () => engine.mod_direct_edit(item_id, "\uD800"));

        Assert.AreEqual("b.txt", row.new_name);
        Assert.IsTrue(row.direct);
        Assert.AreEqual(RESULT.invalid_argument, broken.result);
    }

    [TestMethod]
    public void reports_conflicts_and_issues()
    {
        using TEMP_FOLDER temp = new();
        temp.file("new_a.txt");
        using ENGINE engine = new();
        engine.items_add([temp.file("a.txt"), temp.file("b.txt")], ADD_MODE.as_items);
        PREVIEW preview = engine.preview();

        engine.mod_add(INSERT_NEW);
        engine.mod_direct_edit(preview.rows[1].item_id, "b?.txt");
        preview = engine.preview();

        Assert.AreEqual(STATUS.conflict, preview.rows[0].status);
        Assert.AreEqual(CONFLICT.existing_item, preview.rows[0].conflict);
        Assert.AreEqual(STATUS.error, preview.rows[1].status);
        Assert.AreEqual(NAME_ISSUE.win_forbidden_char, preview.rows[1].errors);
        Assert.AreEqual(1u, preview.conflict);
        Assert.AreEqual(1u, preview.error);
    }

    [TestMethod]
    public void rejects_invalid_json()
    {
        using ENGINE engine = new();

        ENGINE_EXCEPTION error = Assert.ThrowsExactly<ENGINE_EXCEPTION>(() => engine.mod_add("{"));

        Assert.AreEqual(RESULT.invalid_json, error.result);
    }

    [TestMethod]
    public void tracks_history()
    {
        using ENGINE engine = new();

        ENGINE_EXCEPTION empty = Assert.ThrowsExactly<ENGINE_EXCEPTION>(() => engine.undo());
        engine.mod_add(INSERT_NEW);
        (bool can_undo, bool can_redo) added = engine.history_state();
        engine.undo();
        string undone = engine.mods_get();
        (bool can_undo, bool can_redo) after_undo = engine.history_state();
        engine.redo();

        Assert.AreEqual(RESULT.not_found, empty.result);
        Assert.AreEqual((true, false), added);
        Assert.AreEqual("[]", undone);
        Assert.AreEqual((false, true), after_undo);
        Assert.AreNotEqual("[]", engine.mods_get());
    }

    [TestMethod]
    public void reads_and_sets_options()
    {
        using ENGINE engine = new();

        engine.options_set("""{"duplicate":{"enabled":true}}""");
        string options = engine.options_get();

        Assert.IsTrue(options.Contains("\"enabled\":true"), options);
    }
}
