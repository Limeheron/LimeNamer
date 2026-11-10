using System.IO;
using System.Linq;
using System.Threading.Tasks;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace limenamer;

[TestClass]
public sealed class MAIN_VIEW_MODEL_TESTS
{
    [TestMethod]
    public void adds_removes_and_clears_items()
    {
        using MAIN_SCENARIO scenario = new();
        MAIN_VIEW_MODEL main = scenario.main;
        string a = scenario.temp.file("a.txt");
        string b = scenario.temp.file("b.txt");

        main.add_paths([a, b, a, Path.Combine(scenario.temp.path, "\uD800.txt")],
            ADD_MODE.as_items);
        string added = scenario.message;
        bool could_remove = main.remove_command.CanExecute(null);
        main.list.set_selection([scenario.id("a.txt")]);
        bool can_remove = main.remove_command.CanExecute(null);
        main.remove_command.Execute(null);
        string removed = scenario.message;
        main.undo_command.Execute(null);
        int restored = main.list.count_all;
        main.redo_command.Execute(null);
        main.clear_command.Execute(null);

        Assert.AreEqual("2개를 목록에 추가했습니다. 이미 목록에 있는 1개는 건너뛰었습니다. "
            + "1개는 추가하지 못했습니다.", added);
        Assert.IsFalse(could_remove);
        Assert.IsTrue(can_remove);
        Assert.AreEqual("1개 항목을 목록에서 뺐습니다. 실행 취소로 되돌릴 수 있습니다.", removed);
        Assert.AreEqual(2, restored);
        Assert.AreEqual(0, main.list.count_all);
        Assert.AreEqual("목록을 비웠습니다. 실행 취소로 되돌릴 수 있습니다.", scenario.message);
        Assert.IsFalse(main.clear_command.CanExecute(null));
    }

    [TestMethod]
    public async Task blocks_actions_while_editing()
    {
        using MAIN_SCENARIO scenario = new("a.txt");
        MAIN_VIEW_MODEL main = scenario.main;
        scenario.add_mod(LIST_SCENARIO.INSERT_NEW);
        main.mods.open_add();
        main.mods.choose_kind(MOD_KIND.insert);
        string block = "편집 중인 수정을 적용하거나 취소한 뒤 다시 하세요.";

        main.undo();
        string undo = scenario.message;
        main.panel_open = false;
        string panel = scenario.message;
        main.direct_edit(scenario.id("a.txt"), "b.txt");
        string direct = scenario.message;
        await main.rename();

        Assert.IsTrue(main.is_editing);
        Assert.AreEqual("실행 취소를 할 수 없습니다. " + block, undo);
        Assert.AreEqual("수정 패널을 닫을 수 없습니다. " + block, panel);
        Assert.IsTrue(main.panel_open);
        Assert.AreEqual("수정을 편집하는 동안에는 이름을 직접 고칠 수 없습니다. "
            + "적용하거나 취소한 뒤 다시 하세요.", direct);
        Assert.AreEqual("이름을 바꿀 수 없습니다. " + block, scenario.message);
        Assert.IsTrue(scenario.temp.exists("a.txt"));
    }

    [TestMethod]
    public async Task renames_after_confirming_warnings()
    {
        using MAIN_SCENARIO scenario = new("a.txt", "b.txt");
        MAIN_VIEW_MODEL main = scenario.main;
        scenario.add_mod(LIST_SCENARIO.INSERT_NEW);
        main.direct_edit(scenario.id("b.txt"), ".b");
        main.list.filter = VIEW_FILTER.warning;
        scenario.dialogs.answers.Enqueue(false);

        await main.rename();
        bool kept = scenario.temp.exists("a.txt");
        await main.rename();

        DIALOG_CONTENT warning = scenario.dialogs.asked[0];
        Assert.IsTrue(kept);
        Assert.AreEqual(DIALOG_KIND.confirm, warning.kind);
        Assert.AreEqual("경고가 있는 항목이 있습니다", warning.title);
        Assert.AreEqual("경고 1개를 포함해 2개 항목의 이름을 바꿉니다.", warning.message);
        Assert.AreEqual(new DIALOG_LINE(DIALOG_ICON.warning, 2, "b.txt → .b",
            "이름이 점으로 시작합니다. macOS에서는 숨김 파일이 됩니다."), warning.lines.Single());
        Assert.AreEqual(2, scenario.dialogs.asked.Count);
        Assert.IsTrue(scenario.temp.exists("new_a.txt") && scenario.temp.exists(".b"));
        Assert.AreEqual("2개 항목의 이름을 바꿨습니다. "
            + "같은 수정이 다시 적용되지 않도록 수정을 모두 껐습니다.", scenario.message);
        Assert.AreEqual(VIEW_FILTER.all, main.list.filter);
        Assert.IsTrue(main.rename_undo_command.CanExecute(null));
        Assert.AreEqual(false, main.mods.all_enabled_state);
    }

    [TestMethod]
    public async Task refuses_to_rename_with_problems()
    {
        using MAIN_SCENARIO scenario = new("a.txt", "b.txt");
        scenario.temp.file("new_b.txt");
        scenario.main.activated();
        scenario.add_mod(LIST_SCENARIO.INSERT_NEW);

        await scenario.main.rename();

        Assert.AreEqual("이름을 바꾸지 않았습니다. 충돌 1개를 해결해야 합니다. "
            + "첫 번째 문제는 2번 항목입니다.", scenario.message);
        CollectionAssert.AreEqual(new[] { scenario.id("b.txt") },
            scenario.main.list.selected_ids.ToArray());
        Assert.IsTrue(scenario.temp.exists("a.txt"));
    }

    [TestMethod]
    public async Task keeps_succeeded_items_after_failure()
    {
        using MAIN_SCENARIO scenario = new("a.txt", "b.txt");
        scenario.add_mod(LIST_SCENARIO.INSERT_NEW);
        scenario.dialogs.answers.Enqueue(false);

        using (FileStream locked = new(Path.Combine(scenario.temp.path, "a.txt"), FileMode.Open,
                FileAccess.Read, FileShare.None)) {
            await scenario.main.rename();
        }

        DIALOG_CONTENT choice = scenario.dialogs.asked.Single();
        Assert.AreEqual(DIALOG_KIND.choice, choice.kind);
        Assert.AreEqual("일부 항목의 이름을 바꾸지 못했습니다", choice.title);
        Assert.AreEqual("2개 중 1개를 바꿨고 1개는 바꾸지 못했습니다. 처리 방식을 고르세요.",
            choice.message);
        Assert.AreEqual(new DIALOG_LINE(DIALOG_ICON.error, 1, "a.txt → new_a.txt",
            "다른 프로그램이 파일을 사용하고 있습니다."), choice.lines.Single());
        Assert.AreEqual(("전체 되돌리기", "성공한 항목 유지"), (choice.primary, choice.secondary));
        Assert.AreEqual("1개 항목의 이름을 바꿨습니다. 실패한 1개는 그대로 있습니다. "
            + "같은 수정이 다시 적용되지 않도록 수정을 모두 껐습니다.", scenario.message);
        Assert.IsTrue(scenario.temp.exists("a.txt") && scenario.temp.exists("new_b.txt"));
    }

    [TestMethod]
    public async Task reverts_all_after_failure()
    {
        using MAIN_SCENARIO scenario = new("a.txt", "b.txt");
        scenario.add_mod(LIST_SCENARIO.INSERT_NEW);
        scenario.dialogs.answers.Enqueue(true);

        using (FileStream locked = new(Path.Combine(scenario.temp.path, "a.txt"), FileMode.Open,
                FileAccess.Read, FileShare.None)) {
            await scenario.main.rename();
        }

        Assert.AreEqual("모든 항목을 실행 전 이름으로 되돌렸습니다.", scenario.message);
        Assert.IsTrue(scenario.temp.exists("a.txt") && scenario.temp.exists("b.txt"));
        Assert.IsFalse(scenario.main.rename_undo_command.CanExecute(null));
        Assert.AreEqual(true, scenario.main.mods.all_enabled_state);
    }

    [TestMethod]
    public async Task undoes_rename_after_confirmation()
    {
        using MAIN_SCENARIO scenario = new("a.txt", "b.txt");
        scenario.add_mod(LIST_SCENARIO.INSERT_NEW);
        await scenario.main.rename();
        File.Delete(Path.Combine(scenario.temp.path, "new_b.txt"));

        await scenario.main.rename_undo();

        DIALOG_CONTENT confirm = scenario.dialogs.asked[0];
        DIALOG_CONTENT result = scenario.dialogs.asked[1];
        Assert.AreEqual("마지막으로 바꾼 이름을 되돌릴까요?", confirm.title);
        Assert.AreEqual("1개 항목을 바꾸기 전 이름으로 돌립니다. 그 뒤에 목록을 고쳤더라도 "
            + "마지막 실행 기록을 기준으로 되돌립니다. 아래 1개는 되돌릴 수 없어 건너뜁니다.",
            confirm.message);
        Assert.AreEqual(new DIALOG_LINE(DIALOG_ICON.warning, 2, "new_b.txt → b.txt",
            "항목을 찾을 수 없습니다."), confirm.lines.Single());
        Assert.AreEqual(DIALOG_KIND.report, result.kind);
        Assert.AreEqual("일부 항목을 되돌리지 못했습니다", result.title);
        Assert.AreEqual("1개 항목을 실행 전 이름으로 되돌렸습니다. "
            + "건너뛴 1개는 되돌리지 않았습니다.", scenario.message);
        Assert.IsTrue(scenario.temp.exists("a.txt"));
    }

    [TestMethod]
    public void sorts_and_moves_items()
    {
        using MAIN_SCENARIO scenario = new("a.txt", "b.txt", "c.txt");
        MAIN_VIEW_MODEL main = scenario.main;

        main.sort(SORT_KEY.name_natural, true);
        (SORT_KEY key, bool descending) sorted = (main.sort_key, main.sort_descending);
        main.move_rows([scenario.id("c.txt")], scenario.id("a.txt"), true);

        Assert.AreEqual((SORT_KEY.name_natural, true), sorted);
        CollectionAssert.AreEqual(new[] { "b.txt", "a.txt", "c.txt" },
            main.list.rows.Select(row => row.name).ToArray());
        Assert.AreEqual(SORT_KEY.custom, main.sort_key);
        Assert.AreEqual("순서를 바꿨습니다. 정렬은 사용자 지정이 됩니다.", scenario.message);
    }

    [TestMethod]
    public void edits_directly_and_reports_rows()
    {
        using MAIN_SCENARIO scenario = new("a.txt", "b.txt");
        MAIN_VIEW_MODEL main = scenario.main;
        ulong b = scenario.id("b.txt");

        main.direct_edit(b, "b?.txt");
        string edited = scenario.message;
        main.show_row_problems(b);
        string problems = scenario.message;
        File.Delete(Path.Combine(scenario.temp.path, "a.txt"));
        string? missing = main.location_of(scenario.id("a.txt"));

        Assert.AreEqual("2번 항목을 직접 수정했습니다. 수정 목록 맨 아래에 직접 수정으로 들어갑니다.",
            edited);
        Assert.AreEqual("2번 항목: Windows에서 쓸 수 없는 문자가 있습니다. "
            + "(\\ / : * ? \" < > |)", problems);
        Assert.IsNull(missing);
        Assert.AreEqual("항목을 찾을 수 없습니다.", scenario.message);
        Assert.AreEqual(Path.Combine(scenario.temp.path, "b.txt"), main.location_of(b));
    }

    [TestMethod]
    public void applies_duplicate_options()
    {
        using MAIN_SCENARIO scenario = new("a.txt");
        MAIN_VIEW_MODEL main = scenario.main;

        main.duplicate_enabled = true;
        main.duplicate_format = DUPLICATE_FORMAT.custom;
        string custom = scenario.engine.options_get();
        main.duplicate_custom_text = "";
        string unchanged = scenario.engine.options_get();
        main.duplicate_custom_text = "_v";

        Assert.IsTrue(main.duplicate_enabled);
        Assert.IsTrue(custom.Contains("\"custom_text\":\"_copy\""), custom);
        Assert.AreEqual(custom, unchanged);
        Assert.AreEqual(DUPLICATE_FORMAT.custom, main.duplicate_format);
        Assert.IsTrue(scenario.engine.options_get().Contains("\"custom_text\":\"_v\""));
    }

    [TestMethod]
    public void toggles_theme_and_keeps_settings()
    {
        using MAIN_SCENARIO scenario = new();
        MAIN_VIEW_MODEL main = scenario.main;
        string path = Path.Combine(scenario.temp.path, "settings", "settings.json");

        main.toggle_theme(true);
        string light = scenario.message;
        main.toggle_theme(true);
        scenario.settings.show_size = true;
        scenario.settings.example_collapsed = true;
        SETTINGS loaded = SETTINGS.load(path);
        File.WriteAllText(path, "{");
        SETTINGS broken = SETTINGS.load(path);

        Assert.AreEqual("화면 모드를 밝게로 바꿨습니다.", light);
        Assert.AreEqual("화면 모드를 어둡게로 바꿨습니다.", scenario.message);
        Assert.AreEqual(THEME_MODE.dark, loaded.theme_mode);
        Assert.IsTrue(loaded.show_size && loaded.example_collapsed && loaded.show_index);
        Assert.AreEqual(THEME_MODE.system, broken.theme_mode);
        Assert.IsFalse(broken.show_size);
    }
}
