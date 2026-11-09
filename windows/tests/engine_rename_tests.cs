using System.IO;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace limenamer;

[TestClass]
public sealed class ENGINE_RENAME_TESTS
{
    private const string INSERT_NEW =
        """{"kind":"insert","position":{"type":"start"},"value":[{"text":"new_"}]}""";

    [TestMethod]
    public void renames_and_undoes_with_progress()
    {
        using TEMP_FOLDER temp = new();
        using ENGINE engine = new();
        engine.items_add([temp.file("a.txt"), temp.file("b.txt")], ADD_MODE.as_items);
        engine.mod_add(INSERT_NEW);
        PROGRESS progress = new();

        REPORT renamed = engine.rename(progress);
        bool renamed_on_disk = temp.exists("new_a.txt") && temp.exists("new_b.txt");
        (bool available, uint count) undo_state = engine.rename_undo_state();
        REPORT checked_undo = engine.rename_undo_check();
        REPORT undone = engine.rename_undo(null);

        Assert.AreEqual(2u, renamed.total);
        Assert.AreEqual(2u, renamed.succeeded);
        Assert.IsFalse(renamed.needs_choice);
        Assert.AreEqual(0, renamed.problems.Count);
        Assert.AreEqual((2u, 2u), progress.read());
        Assert.IsTrue(renamed_on_disk);
        Assert.AreEqual((true, 2u), undo_state);
        Assert.AreEqual(0u, checked_undo.skipped);
        Assert.AreEqual(2u, undone.succeeded);
        Assert.IsTrue(temp.exists("a.txt") && temp.exists("b.txt"));
        Assert.AreEqual((false, 0u), engine.rename_undo_state());
    }

    [TestMethod]
    public void refuses_to_rename_with_conflicts()
    {
        using TEMP_FOLDER temp = new();
        temp.file("new_a.txt");
        using ENGINE engine = new();
        engine.items_add([temp.file("a.txt")], ADD_MODE.as_items);
        engine.mod_add(INSERT_NEW);

        ENGINE_EXCEPTION error = Assert.ThrowsExactly<ENGINE_EXCEPTION>(
            () => engine.rename(null));

        Assert.AreEqual(RESULT.not_ready, error.result);
    }

    [TestMethod]
    public void reports_failures_and_resolves()
    {
        using TEMP_FOLDER temp = new();
        using ENGINE engine = new();
        string a = temp.file("a.txt");
        engine.items_add([a, temp.file("b.txt")], ADD_MODE.as_items);
        ulong a_id = engine.preview().rows[0].item_id;
        engine.mod_add(INSERT_NEW);

        REPORT failed;
        using (FileStream locked = new(a, FileMode.Open, FileAccess.Read, FileShare.None)) {
            failed = engine.rename(null);
        }
        REPORT resolved = engine.rename_resolve(RESOLVE_CHOICE.revert_all, null);

        Assert.IsTrue(failed.needs_choice);
        Assert.AreEqual(1u, failed.failed);
        Assert.AreEqual(1, failed.problems.Count);
        REPORT_ITEM problem = failed.problems[0];
        Assert.AreEqual(a_id, problem.item_id);
        Assert.AreEqual(a, problem.from_path);
        Assert.AreEqual(Path.Combine(temp.path, "new_a.txt"), problem.to_path);
        Assert.AreEqual(a, problem.current_path);
        Assert.AreEqual(STAGE.run, problem.stage);
        Assert.AreEqual(FAILURE.in_use, problem.failure);
        Assert.AreNotEqual(0, problem.os_error);
        Assert.IsFalse(resolved.needs_choice);
        Assert.IsTrue(temp.exists("a.txt") && temp.exists("b.txt"));
        Assert.IsFalse(temp.exists("new_b.txt"));
    }

    [TestMethod]
    public void resolve_without_waiting_run_is_not_found()
    {
        using ENGINE engine = new();

        ENGINE_EXCEPTION error = Assert.ThrowsExactly<ENGINE_EXCEPTION>(
            () => engine.rename_resolve(RESOLVE_CHOICE.keep_succeeded, null));

        Assert.AreEqual(RESULT.not_found, error.result);
    }
}
