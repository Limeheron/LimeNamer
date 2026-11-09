using System.Linq;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace limenamer;

[TestClass]
public sealed class MOD_LIST_VIEW_MODEL_TESTS
{
    [TestMethod]
    public void loads_cards_with_summaries()
    {
        using MOD_PANEL_SCENARIO scenario = new("a.txt", "b.txt", "c.txt");
        ulong a = scenario.items.id("a.txt");
        ulong c = scenario.items.id("c.txt");
        scenario.engine.mod_add(LIST_SCENARIO.INSERT_NEW);
        scenario.engine.mod_direct_edit(scenario.items.id("b.txt"), "x.txt");
        scenario.engine.mod_add($$"""
            {"kind":"insert","position":{"type":"end"},"value":[{"text":"_k"}],
                "scope":"items","targets":[{{a}},{{c}}]}
            """);
        scenario.engine.items_remove([c]);

        scenario.load();

        CollectionAssert.AreEqual(new[] {
            "이름 맨 앞에 \"new_\" 넣기",
            "2번 항목을 \"x.txt\"로 직접 수정",
            "이름 맨 끝에 \"_k\" 넣기 (1개)",
        }, scenario.mods.cards.Select(card => card.summary).ToArray());
        Assert.IsTrue(scenario.mods.cards[1].direct);
        Assert.AreEqual(true, scenario.mods.all_enabled_state);
        Assert.AreEqual(3, scenario.mods.enabled_count);
    }

    [TestMethod]
    public void toggles_and_removes_mods()
    {
        using MOD_PANEL_SCENARIO scenario = new("a.txt");
        scenario.engine.mod_add(LIST_SCENARIO.INSERT_NEW);
        scenario.engine.mod_add(MOD_PANEL_SCENARIO.INSERT_OLD);
        scenario.load();
        MOD_LIST_VIEW_MODEL mods = scenario.mods;

        mods.set_enabled(mods.cards[0], false);
        bool? mixed = mods.all_enabled_state;
        mods.toggle_all();
        bool? all_on = mods.all_enabled_state;
        mods.toggle_all();
        bool? all_off = mods.all_enabled_state;
        mods.remove(mods.cards[1]);

        Assert.IsNull(mixed);
        Assert.AreEqual(true, all_on);
        Assert.AreEqual(false, all_off);
        Assert.AreEqual(1, mods.cards.Count);
        CollectionAssert.AreEqual(new[] {
            "수정을 껐습니다.",
            "모든 수정을 켰습니다.",
            "모든 수정을 껐습니다.",
            "수정을 지웠습니다. 실행 취소로 되돌릴 수 있습니다.",
        }, scenario.messages);
    }

    [TestMethod]
    public void reorders_cards()
    {
        using MOD_PANEL_SCENARIO scenario = new("a.txt");
        ulong first = scenario.engine.mod_add(LIST_SCENARIO.INSERT_NEW);
        ulong second = scenario.engine.mod_add(MOD_PANEL_SCENARIO.INSERT_OLD);
        scenario.load();
        MOD_LIST_VIEW_MODEL mods = scenario.mods;

        Assert.IsFalse(mods.can_drop(first, first, true));
        Assert.IsFalse(mods.can_drop(first, second, false));
        Assert.IsTrue(mods.drop(second, first, false));

        CollectionAssert.AreEqual(new[] { second, first },
            mods.cards.Select(card => card.id).ToArray());
        Assert.AreEqual("new_old_a.txt", scenario.first_new_name());
        CollectionAssert.AreEqual(new[] { "수정 순서를 바꿨습니다." }, scenario.messages);
    }

    [TestMethod]
    public void adds_mod_from_add_page()
    {
        using MOD_PANEL_SCENARIO scenario = new("a.JPG");
        MOD_LIST_VIEW_MODEL mods = scenario.mods;

        mods.open_add();
        mods.add_part = PART.ext;
        KIND_CARD insert = mods.kind_cards[0];
        mods.choose_kind(MOD_KIND.replace);
        MOD_PAGE editing = mods.page;
        scenario.editor.value = [new VALUE_PIECE { text = "jpeg" }];
        bool applied = mods.apply();

        Assert.AreEqual(new KIND_CARD(MOD_KIND.insert, "넣기", "원하는 위치에 텍스트, 번호, 날짜",
            "IMG_0001.JPG", "IMG_0001.JPG.bak"), insert);
        Assert.AreEqual(MOD_PAGE.edit, editing);
        Assert.IsTrue(applied);
        Assert.AreEqual(MOD_PAGE.list, mods.page);
        Assert.AreEqual("확장자를 \"jpeg\"로 바꾸기", mods.cards[0].summary);
        Assert.AreEqual("a.jpeg", scenario.first_new_name());
        CollectionAssert.AreEqual(new[] { "수정을 추가했습니다." }, scenario.messages);
    }

    [TestMethod]
    public void cancels_with_messages()
    {
        using MOD_PANEL_SCENARIO scenario = new("a.txt");
        scenario.engine.mod_add(LIST_SCENARIO.INSERT_NEW);
        scenario.load();
        MOD_LIST_VIEW_MODEL mods = scenario.mods;

        mods.open_add();
        mods.choose_kind(MOD_KIND.delete);
        mods.cancel();
        mods.open_card(mods.cards[0]);
        mods.cancel();
        mods.open_card(mods.cards[0]);
        scenario.editor.value = [new VALUE_PIECE { text = "x_" }];
        mods.cancel();

        CollectionAssert.AreEqual(new[] {
            "새 수정을 추가하지 않고 닫았습니다.",
            "편집한 내용을 적용하지 않고 닫았습니다.",
        }, scenario.messages);
        Assert.AreEqual("new_a.txt", scenario.first_new_name());
        Assert.AreEqual(MOD_PAGE.list, mods.page);
    }
}
