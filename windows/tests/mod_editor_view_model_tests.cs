using System.Collections.Generic;
using System.Linq;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace limenamer;

[TestClass]
public sealed class MOD_EDITOR_VIEW_MODEL_TESTS
{
    [TestMethod]
    public void reports_problems_on_apply_and_rechecks()
    {
        using MOD_PANEL_SCENARIO scenario = new("a.txt");
        scenario.mods.open_add();
        scenario.mods.add_kind = MOD_KIND.replace;
        scenario.mods.choose_position(POSITION_TYPE.all);
        MOD_EDITOR_VIEW_MODEL editor = scenario.editor;
        editor.position_type = POSITION_TYPE.text;

        bool applied = scenario.mods.apply();
        string find_problem = editor.find_text_problem;
        editor.find_text = "a";

        Assert.IsFalse(applied);
        Assert.AreEqual(MOD_PAGE.edit, scenario.mods.page);
        Assert.AreEqual("찾을 텍스트를 입력하세요.", find_problem);
        Assert.AreEqual("", editor.find_text_problem);
        Assert.AreEqual("새 값을 입력하세요.", editor.value_problem);
        Assert.AreEqual(MOD_FIELD.find_text, scenario.focused);
        CollectionAssert.AreEqual(new[] {
            "적용하지 못했습니다. 찾을 텍스트를 입력하세요. 빨간 칸 2개를 확인하세요.",
        }, scenario.messages);
    }

    [TestMethod]
    public void builds_summary_and_example()
    {
        using MOD_PANEL_SCENARIO scenario = new("a.txt");
        scenario.mods.open_add();
        scenario.mods.add_kind = MOD_KIND.insert;
        scenario.mods.choose_position(POSITION_TYPE.end);
        MOD_EDITOR_VIEW_MODEL editor = scenario.editor;

        POSITION_TYPE initial = editor.position_type;
        editor.value = [new VALUE_PIECE { text = "_x" }];

        Assert.AreEqual(POSITION_TYPE.end, initial);
        Assert.AreEqual("이름 맨 끝에 \"_x\" 넣기", editor.summary);
        Assert.IsTrue(editor.has_example);
        Assert.AreEqual("1번 항목", editor.example_index_text);
        Assert.AreEqual(("a", "", ".txt"),
            (editor.example_before_head, editor.example_before_mark, editor.example_before_tail));
        Assert.AreEqual(("a", "_x", ".txt"),
            (editor.example_after_head, editor.example_after_mark, editor.example_after_tail));
        Assert.AreEqual("", editor.example_state_text);
    }

    [TestMethod]
    public void edits_mods_with_examples_before_them()
    {
        using MOD_PANEL_SCENARIO scenario = new("a.txt");
        scenario.engine.mod_add(LIST_SCENARIO.INSERT_NEW);
        scenario.engine.mod_add(MOD_PANEL_SCENARIO.INSERT_OLD);
        scenario.load();
        MOD_LIST_VIEW_MODEL mods = scenario.mods;
        MOD_EDITOR_VIEW_MODEL editor = scenario.editor;

        mods.open_card(mods.cards[1]);
        string second_before = before_name(editor);
        mods.cancel();
        mods.open_card(mods.cards[0]);
        string first_before = before_name(editor);
        editor.value = [new VALUE_PIECE { text = "x_" }];
        bool applied = mods.apply();

        Assert.AreEqual("new_a.txt", second_before);
        Assert.AreEqual("a.txt", first_before);
        Assert.IsTrue(applied);
        Assert.AreEqual("이름 맨 앞에 \"x_\" 넣기", mods.cards[0].summary);
        Assert.AreEqual("old_x_a.txt", scenario.first_new_name());
        CollectionAssert.AreEqual(new[] { "수정을 고쳤습니다." }, scenario.messages);
    }

    [TestMethod]
    public void designates_targets_for_chosen_items()
    {
        using MOD_PANEL_SCENARIO scenario = new("a.txt", "b.txt", "c.txt");
        ulong b = scenario.items.id("b.txt");
        ITEM_LIST_VIEW_MODEL list = scenario.items.list;
        list.set_selection([scenario.items.id("a.txt")]);
        scenario.mods.open_add();
        scenario.mods.add_kind = MOD_KIND.insert;
        scenario.mods.choose_position(POSITION_TYPE.end);
        MOD_EDITOR_VIEW_MODEL editor = scenario.editor;
        editor.value = [new VALUE_PIECE { text = "_k" }];

        editor.scope = SCOPE.items;
        bool designating = list.designating;
        int initial = list.checked_ids.Count;
        list.toggle_check(b, false);
        string summary = editor.summary;
        bool applied = scenario.mods.apply();

        Assert.IsTrue(designating);
        Assert.AreEqual(0, initial);
        Assert.AreEqual("이름 맨 끝에 \"_k\" 넣기 (1개)", summary);
        Assert.IsTrue(applied);
        Assert.IsFalse(list.designating);
        CollectionAssert.AreEqual(new[] { "a.txt", "b_k.txt", "c.txt" },
            scenario.engine.preview().rows.Select(row => row.new_name).ToArray());
    }

    [TestMethod]
    public void reports_target_problems()
    {
        using MOD_PANEL_SCENARIO scenario = new("a.txt", "b.txt", "c.txt");
        ITEM_LIST_VIEW_MODEL list = scenario.items.list;
        scenario.mods.open_add();
        scenario.mods.add_kind = MOD_KIND.insert;
        scenario.mods.choose_position(POSITION_TYPE.end);
        MOD_EDITOR_VIEW_MODEL editor = scenario.editor;
        editor.value = [new VALUE_PIECE { text = "_k" }];
        editor.scope = SCOPE.items;

        bool applied = scenario.mods.apply();
        string empty = editor.targets_problem;
        list.number_text = "9";
        string range = editor.targets_problem;
        list.toggle_check(scenario.items.id("a.txt"), false);

        Assert.IsFalse(applied);
        Assert.AreEqual("적용할 항목을 체크하거나 번호를 입력하세요.", empty);
        Assert.AreEqual("번호는 1부터 3까지만 쓸 수 있습니다.", range);
        Assert.AreEqual("", editor.targets_problem);
        Assert.AreEqual("이름 맨 끝에 \"_k\" 넣기 (1개)", editor.summary);
    }

    [TestMethod]
    public void offers_choices_and_token_settings()
    {
        using MOD_PANEL_SCENARIO scenario = new("a.txt");
        MOD_EDITOR_VIEW_MODEL editor = scenario.editor;
        scenario.mods.open_add();
        scenario.mods.add_kind = MOD_KIND.delete;
        scenario.mods.choose_position(POSITION_TYPE.all);
        POSITION_TYPE delete_first = editor.position_tiles[0].type;
        editor.position_type = POSITION_TYPE.between;
        int between_occurrences = editor.occurrence_choices.Count;
        scenario.mods.cancel();

        scenario.mods.open_add();
        scenario.mods.add_kind = MOD_KIND.insert;
        scenario.mods.choose_position(POSITION_TYPE.end);
        POSITION_TYPE insert_first = editor.position_tiles[0].type;
        bool before_tokens = editor.uses_number;
        editor.value = [
            new VALUE_PIECE { text = "a" },
            new VALUE_PIECE { text = "b" },
            new VALUE_PIECE { text = "" },
            new VALUE_PIECE { token = TOKEN_KIND.number },
            new VALUE_PIECE { token = TOKEN_KIND.modified_date },
        ];
        editor.modified_date_choice = "";
        editor.modified_date_custom = "yyyy";
        scenario.mods.apply();
        string stored = scenario.engine.mods_get();

        Assert.AreEqual(POSITION_TYPE.all, delete_first);
        Assert.AreEqual(4, between_occurrences);
        Assert.AreEqual(POSITION_TYPE.start, insert_first);
        Assert.IsFalse(before_tokens);
        Assert.AreEqual(3, editor.value.Count);
        Assert.IsTrue(editor.uses_number && editor.uses_modified_date);
        Assert.IsTrue(stored.Contains("\"text\":\"ab\""), stored);
        Assert.IsTrue(stored.Contains("\"digits\":3"), stored);
        Assert.IsTrue(stored.Contains("\"format\":\"yyyy\""), stored);
    }

    [TestMethod]
    public void offers_position_samples()
    {
        using MOD_PANEL_SCENARIO scenario = new("a.txt");
        MOD_EDITOR_VIEW_MODEL editor = scenario.editor;
        IReadOnlyList<POSITION_CHOICE> insert = editor.position_choices(MOD_KIND.insert);
        IReadOnlyList<POSITION_CHOICE> delete = editor.position_choices(MOD_KIND.delete);
        IReadOnlyList<POSITION_CHOICE> replace = editor.position_choices(MOD_KIND.replace);
        IReadOnlyList<POSITION_CHOICE> format = editor.position_choices(MOD_KIND.format);

        CollectionAssert.AreEqual(new[] {
            POSITION_TYPE.start, POSITION_TYPE.end, POSITION_TYPE.after_nth,
            POSITION_TYPE.before_nth_from_end, POSITION_TYPE.before_text, POSITION_TYPE.after_text,
        }, insert.Select(choice => choice.type).ToArray());
        CollectionAssert.AreEqual(new[] {
            POSITION_TYPE.all, POSITION_TYPE.from_nth, POSITION_TYPE.first_n, POSITION_TYPE.last_n,
            POSITION_TYPE.text, POSITION_TYPE.between,
        }, delete.Select(choice => choice.type).ToArray());
        Assert.AreEqual("맨 앞", insert[0].label);
        Assert.AreEqual("이름 앞에 붙입니다.", insert[0].description);
        Assert.AreEqual("앞에서 N글자 뒤", insert[2].label);
        CollectionAssert.AreEqual(new[] {
            piece("2024_", SAMPLE_STYLE.mark), piece("IMG_0001.JPG", SAMPLE_STYLE.plain),
        }, insert[0].after.ToArray());
        CollectionAssert.AreEqual(new[] {
            piece("", SAMPLE_STYLE.caret), piece("IMG_0001.JPG", SAMPLE_STYLE.plain),
        }, insert[0].tile.ToArray());
        CollectionAssert.AreEqual(new[] {
            piece("IMG_0001", SAMPLE_STYLE.plain), piece("", SAMPLE_STYLE.caret),
            piece(".JPG", SAMPLE_STYLE.plain),
        }, insert[1].tile.ToArray());
        CollectionAssert.AreEqual(new[] {
            piece("IMG_", SAMPLE_STYLE.plain), piece("", SAMPLE_STYLE.caret),
            piece("0001", SAMPLE_STYLE.anchor), piece(".JPG", SAMPLE_STYLE.plain),
        }, insert[4].tile.ToArray());
        CollectionAssert.AreEqual(new[] {
            piece("IMG", SAMPLE_STYLE.anchor), piece("", SAMPLE_STYLE.caret),
            piece("_0001.JPG", SAMPLE_STYLE.plain),
        }, insert[5].tile.ToArray());
        CollectionAssert.AreEqual(new[] {
            piece("IMG_", SAMPLE_STYLE.plain), piece("0001", SAMPLE_STYLE.removed),
            piece(".JPG", SAMPLE_STYLE.plain),
        }, delete[3].before.ToArray());
        CollectionAssert.AreEqual(new[] { piece("IMG_.JPG", SAMPLE_STYLE.plain) },
            delete[3].after.ToArray());
        CollectionAssert.AreEqual(new[] {
            piece("IMG_", SAMPLE_STYLE.plain), piece("0001", SAMPLE_STYLE.mark),
            piece(".JPG", SAMPLE_STYLE.plain),
        }, delete[3].tile.ToArray());
        CollectionAssert.AreEqual(new[] {
            piece("I", SAMPLE_STYLE.plain), piece("MG_", SAMPLE_STYLE.mark),
            piece("0001.JPG", SAMPLE_STYLE.plain),
        }, delete[1].tile.ToArray());
        CollectionAssert.AreEqual(new[] {
            piece("IMG_", SAMPLE_STYLE.mark_anchor), piece("0001.JPG", SAMPLE_STYLE.plain),
        }, delete[4].tile.ToArray());
        CollectionAssert.AreEqual(new[] {
            piece("IMG", SAMPLE_STYLE.anchor), piece("_0001", SAMPLE_STYLE.mark),
            piece(".JPG", SAMPLE_STYLE.plain),
        }, delete[5].tile.ToArray());
        CollectionAssert.AreEqual(new[] {
            piece("제주_", SAMPLE_STYLE.mark), piece(".JPG", SAMPLE_STYLE.plain),
        }, replace[0].after.ToArray());
        Assert.IsFalse(format[0].shows_after);
        CollectionAssert.AreEqual(format[3].tile.ToArray(), format[3].before.ToArray());
        Assert.AreSame(insert, editor.position_choices(MOD_KIND.insert));
    }

    [TestMethod]
    public void starts_inserts_at_the_front()
    {
        using MOD_PANEL_SCENARIO scenario = new("a.txt");
        scenario.mods.open_add();
        scenario.mods.choose_position(POSITION_TYPE.start);
        scenario.editor.value = [new VALUE_PIECE { text = "x_" }];
        scenario.mods.apply();

        Assert.AreEqual("x_a.txt", scenario.first_new_name());
        Assert.AreEqual(POSITION_TYPE.start,
            scenario.editor.position_choices(MOD_KIND.insert)[0].type);
    }

    private static SAMPLE_PIECE piece(string text, SAMPLE_STYLE style)
    {
        return new SAMPLE_PIECE(text, style);
    }

    private static string before_name(MOD_EDITOR_VIEW_MODEL editor)
    {
        return editor.example_before_head + editor.example_before_mark
            + editor.example_before_tail;
    }
}
