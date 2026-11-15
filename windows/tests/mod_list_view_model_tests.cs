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
        KIND_CARD insert = mods.kind_header;
        string badge = mods.badge_text;
        mods.add_kind = MOD_KIND.replace;
        mods.choose_position(POSITION_TYPE.all);
        MOD_PAGE editing = mods.page;
        string apply_label = mods.apply_label;
        scenario.editor.part = PART.ext;
        scenario.editor.value = [new VALUE_PIECE { text = "jpeg" }];
        bool applied = mods.apply();

        Assert.AreEqual(new KIND_CARD(MOD_KIND.insert, "넣기",
            "원하는 위치에 텍스트, 번호, 날짜 넣기", "IMG_0001.JPG", "2024_IMG_0001.JPG"), insert);
        Assert.AreEqual("신규", badge);
        Assert.AreEqual(MOD_PAGE.edit, editing);
        Assert.AreEqual("추가", apply_label);
        Assert.IsTrue(applied);
        Assert.AreEqual(MOD_PAGE.list, mods.page);
        Assert.AreEqual("확장자를 \"jpeg\"로 바꾸기", mods.cards[0].summary);
        Assert.AreEqual("a.jpeg", scenario.first_new_name());
        CollectionAssert.AreEqual(new[] { "수정을 추가했습니다." }, scenario.messages);
    }

    [TestMethod]
    public void goes_back_to_the_previous_page()
    {
        using MOD_PANEL_SCENARIO scenario = new("a.txt");
        scenario.engine.mod_add(LIST_SCENARIO.INSERT_NEW);
        scenario.load();
        MOD_LIST_VIEW_MODEL mods = scenario.mods;

        mods.open_add();
        mods.add_kind = MOD_KIND.delete;
        mods.choose_position(POSITION_TYPE.first_n);
        string new_label = mods.back_label;
        mods.go_back();
        MOD_PAGE after_new = mods.page;
        MOD_KIND kept_kind = mods.add_kind;
        string add_label = mods.add_back_label;
        mods.go_back();
        MOD_PAGE after_add = mods.page;
        mods.open_card(mods.cards[0]);
        string edit_label = mods.back_label;
        mods.go_back();

        Assert.AreEqual("수정 추가", new_label);
        Assert.AreEqual(MOD_PAGE.add, after_new);
        Assert.AreEqual(MOD_KIND.delete, kept_kind);
        Assert.AreEqual("수정 목록", add_label);
        Assert.AreEqual(MOD_PAGE.list, after_add);
        Assert.AreEqual("수정 목록", edit_label);
        Assert.AreEqual(MOD_PAGE.list, mods.page);
        Assert.AreEqual(0, scenario.messages.Count);
    }

    [TestMethod]
    public void switches_kinds_of_new_and_existing_mods()
    {
        using MOD_PANEL_SCENARIO scenario = new("a.txt");
        scenario.engine.mod_add(LIST_SCENARIO.INSERT_NEW);
        scenario.load();
        MOD_LIST_VIEW_MODEL mods = scenario.mods;

        mods.open_add();
        mods.choose_position(POSITION_TYPE.end);
        mods.switch_kind(MOD_KIND.delete);
        MOD_PAGE new_page = mods.page;
        bool new_active = scenario.editor.is_active;
        string new_back = mods.add_back_label;
        mods.go_back();
        MOD_PAGE after_new_back = mods.page;

        MOD_CARD card = mods.cards[0];
        mods.open_card(card);
        string badge = mods.badge_text;
        string apply_label = mods.apply_label;
        mods.switch_kind(MOD_KIND.insert);
        MOD_PAGE same_kind = mods.page;
        mods.switch_kind(MOD_KIND.replace);
        string edit_back = mods.add_back_label;
        string kept_badge = mods.badge_text;
        mods.go_back();
        MOD_KIND restored = scenario.editor.kind;
        mods.switch_kind(MOD_KIND.replace);
        mods.choose_position(POSITION_TYPE.all);
        ulong editing = scenario.editor.editing_id;
        scenario.editor.value = [new VALUE_PIECE { text = "b" }];
        bool applied = mods.apply();

        Assert.AreEqual(MOD_PAGE.add, new_page);
        Assert.IsFalse(new_active);
        Assert.AreEqual("수정 목록", new_back);
        Assert.AreEqual(MOD_PAGE.list, after_new_back);
        Assert.AreEqual("편집 중", badge);
        Assert.AreEqual("적용", apply_label);
        Assert.AreEqual(MOD_PAGE.edit, same_kind);
        Assert.AreEqual("편집", edit_back);
        Assert.AreEqual("편집 중", kept_badge);
        Assert.AreEqual(MOD_KIND.insert, restored);
        Assert.AreEqual(card.id, editing);
        Assert.IsTrue(applied);
        Assert.AreEqual(1, mods.cards.Count);
        Assert.AreEqual(card.id, mods.cards[0].id);
        Assert.AreEqual(MOD_KIND.replace, mods.cards[0].mod.kind);
        Assert.AreEqual("b.txt", scenario.first_new_name());
        CollectionAssert.AreEqual(new[] { "수정을 고쳤습니다." }, scenario.messages);
    }

    [TestMethod]
    public void cancels_a_switched_kind_with_the_original_kept()
    {
        using MOD_PANEL_SCENARIO scenario = new("a.txt");
        scenario.engine.mod_add(LIST_SCENARIO.INSERT_NEW);
        scenario.load();
        MOD_LIST_VIEW_MODEL mods = scenario.mods;

        mods.open_card(mods.cards[0]);
        mods.switch_kind(MOD_KIND.delete);
        mods.choose_position(POSITION_TYPE.all);
        mods.cancel();

        Assert.AreEqual(MOD_KIND.insert, mods.cards[0].mod.kind);
        Assert.AreEqual("new_a.txt", scenario.first_new_name());
        CollectionAssert.AreEqual(new[] { "편집을 취소했습니다." }, scenario.messages);
    }

    [TestMethod]
    public void turns_an_edited_direct_edit_into_a_plain_mod()
    {
        using MOD_PANEL_SCENARIO scenario = new("a.txt");
        scenario.engine.mod_direct_edit(scenario.items.id("a.txt"), "b.txt");
        scenario.load();
        MOD_LIST_VIEW_MODEL mods = scenario.mods;

        mods.open_card(mods.cards[0]);
        string summary = scenario.editor.summary;
        scenario.editor.value = [new VALUE_PIECE { text = "c.txt" }];
        mods.apply();

        Assert.AreEqual("이름과 확장자를 \"b.txt\"로 바꾸기 (1개)", summary);
        Assert.IsFalse(mods.cards[0].direct);
        Assert.AreEqual("이름과 확장자를 \"c.txt\"로 바꾸기 (1개)", mods.cards[0].summary);
        Assert.AreEqual("c.txt", scenario.first_new_name());
    }

    [TestMethod]
    public void cancels_with_messages()
    {
        using MOD_PANEL_SCENARIO scenario = new("a.txt");
        scenario.engine.mod_add(LIST_SCENARIO.INSERT_NEW);
        scenario.load();
        MOD_LIST_VIEW_MODEL mods = scenario.mods;

        mods.open_add();
        mods.add_kind = MOD_KIND.delete;
        mods.choose_position(POSITION_TYPE.all);
        mods.cancel();
        mods.open_card(mods.cards[0]);
        mods.cancel();
        mods.open_card(mods.cards[0]);
        scenario.editor.value = [new VALUE_PIECE { text = "x_" }];
        mods.cancel();

        CollectionAssert.AreEqual(new[] {
            "새 수정을 취소했습니다.",
            "편집을 취소했습니다.",
        }, scenario.messages);
        Assert.AreEqual("new_a.txt", scenario.first_new_name());
        Assert.AreEqual(MOD_PAGE.list, mods.page);
    }
}
