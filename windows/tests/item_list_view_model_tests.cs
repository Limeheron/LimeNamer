using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace limenamer;

[TestClass]
public sealed class ITEM_LIST_VIEW_MODEL_TESTS
{
    [TestMethod]
    public void builds_rows_from_preview()
    {
        using LIST_SCENARIO scenario = new();
        string a = scenario.temp.file("a.txt", "abc");
        string photos = scenario.temp.folder("photos");
        scenario.engine.items_add([a, photos], ADD_MODE.as_items);
        scenario.engine.mod_add(LIST_SCENARIO.INSERT_NEW);

        scenario.load();
        ITEM_ROW file = scenario.list.rows[0];
        ITEM_ROW folder = scenario.list.rows[1];

        Assert.AreEqual(2, scenario.list.rows.Count);
        Assert.AreEqual(1u, file.index);
        Assert.AreEqual(a, file.full_path);
        Assert.AreEqual(("", "new_", "a.txt"),
            (file.new_name_head, file.new_name_mark, file.new_name_tail));
        Assert.AreEqual("정상", file.status_text);
        Assert.AreEqual("3B", file.size_text);
        Assert.IsTrue(Regex.IsMatch(file.modified_text, @"^\d{4}-\d{2}-\d{2} \d{2}:\d{2}$"));
        Assert.IsTrue(folder.is_dir);
        Assert.AreEqual("—", folder.size_text);
        Assert.AreEqual("항목 2개 · 선택 0개", scenario.list.header_text);
    }

    [TestMethod]
    public void filters_rows_by_status()
    {
        using LIST_SCENARIO scenario = STATUS_SCENARIO.create();
        ITEM_LIST_VIEW_MODEL list = scenario.list;

        Assert.AreEqual((4u, 1u, 1u, 1u),
            (list.count_changed, list.count_error, list.count_conflict, list.count_warning));
        list.filter = VIEW_FILTER.error;
        CollectionAssert.AreEqual(new[] { "c.txt" }, scenario.visible_names());
        Assert.IsFalse(list.can_drag);
        Assert.AreEqual("항목 5개 · 선택 0개 · 오류 항목만 보는 중", list.header_text);
        list.filter = VIEW_FILTER.conflict;
        CollectionAssert.AreEqual(new[] { "b.txt" }, scenario.visible_names());
        list.filter = VIEW_FILTER.warning;
        CollectionAssert.AreEqual(new[] { "d.txt" }, scenario.visible_names());
        list.filter = VIEW_FILTER.changed;
        CollectionAssert.AreEqual(new[] { "a.txt", "b.txt", "c.txt", "d.txt" },
            scenario.visible_names());
    }

    [TestMethod]
    public void reports_empty_states()
    {
        using LIST_SCENARIO empty = new();
        using LIST_SCENARIO scenario = new("a.txt");

        bool empty_list = empty.list.shows_empty_list;
        bool filled = scenario.list.shows_empty_list || scenario.list.shows_empty_filter;
        scenario.list.filter = VIEW_FILTER.error;

        Assert.IsTrue(empty_list);
        Assert.IsFalse(empty.list.shows_empty_filter);
        Assert.IsFalse(filled);
        Assert.IsTrue(scenario.list.shows_empty_filter);
        Assert.AreEqual("오류 항목이 없습니다", scenario.list.empty_filter_title);
    }

    [TestMethod]
    public void keeps_selection_across_loads()
    {
        using LIST_SCENARIO scenario = new("a.txt", "b.txt");
        ulong a = scenario.id("a.txt");
        ulong b = scenario.id("b.txt");
        List<ulong> requested = new();
        scenario.list.selection_requested += ids => requested = ids.ToList();

        scenario.list.set_selection([a, b]);
        scenario.engine.items_remove([a]);
        scenario.load();

        CollectionAssert.AreEquivalent(new[] { b }, scenario.list.selected_ids.ToArray());
        CollectionAssert.AreEquivalent(new[] { b }, requested);
    }

    [TestMethod]
    public void reveals_hidden_rows()
    {
        using LIST_SCENARIO scenario = STATUS_SCENARIO.create();
        ulong a = scenario.id("a.txt");
        ulong revealed = 0;
        scenario.list.reveal_requested += id => revealed = id;
        scenario.list.filter = VIEW_FILTER.error;

        scenario.list.reveal(a);

        Assert.AreEqual(VIEW_FILTER.all, scenario.list.filter);
        CollectionAssert.AreEquivalent(new[] { a }, scenario.list.selected_ids.ToArray());
        Assert.AreEqual(a, revealed);
    }

    [TestMethod]
    public void moves_dragged_rows()
    {
        using LIST_SCENARIO scenario = new("a.txt", "b.txt", "c.txt", "d.txt");
        ulong a = scenario.id("a.txt");
        ulong b = scenario.id("b.txt");
        ulong c = scenario.id("c.txt");
        ulong d = scenario.id("d.txt");
        ITEM_LIST_VIEW_MODEL list = scenario.list;
        list.set_selection([c, a]);

        CollectionAssert.AreEqual(new[] { a, c }, list.drag_ids(c).ToArray());
        CollectionAssert.AreEqual(new[] { b }, list.drag_ids(b).ToArray());
        Assert.IsFalse(list.can_drop([b], a, true));
        Assert.IsFalse(list.can_drop([a, c], c, true));
        Assert.IsTrue(list.drop([a, c], d, true));
        scenario.load();

        CollectionAssert.AreEqual(new[] { "b.txt", "d.txt", "a.txt", "c.txt" },
            scenario.visible_names());
        list.filter = VIEW_FILTER.changed;
        Assert.IsFalse(list.can_drop([b], d, true));
    }

    [TestMethod]
    public void designates_with_checks_and_numbers()
    {
        using LIST_SCENARIO scenario = new("a.txt", "b.txt", "c.txt", "d.txt", "e.txt");
        ITEM_LIST_VIEW_MODEL list = scenario.list;
        ulong a = scenario.id("a.txt");
        ulong d = scenario.id("d.txt");

        list.begin_designation([scenario.id("b.txt")]);
        string initial = list.number_text;
        bool a_dimmed = list.rows[0].is_dimmed;
        list.toggle_check(a, false);
        list.toggle_check(d, true);
        string ranged = list.number_text;
        bool? partial = list.check_all_state;
        list.number_text = "2, 4-5";
        string[] typed = checked_names(list);
        list.number_text = "9";
        string error = list.number_error;
        string[] kept = checked_names(list);
        list.check_all(true);

        Assert.AreEqual("2", initial);
        Assert.IsTrue(a_dimmed);
        Assert.AreEqual("1-4", ranged);
        Assert.IsNull(partial);
        CollectionAssert.AreEqual(new[] { "b.txt", "d.txt", "e.txt" }, typed);
        Assert.AreEqual("번호는 1부터 5까지만 쓸 수 있습니다.", error);
        CollectionAssert.AreEqual(typed, kept);
        Assert.AreEqual("1-5", list.number_text);
        Assert.AreEqual(true, list.check_all_state);
        Assert.AreEqual("항목 5개 · 체크 5개", list.header_text);

        list.end_designation();

        Assert.IsFalse(list.rows.Any(row => row.is_dimmed || row.is_checked));
        Assert.AreEqual("", list.number_text);
    }

    [TestMethod]
    public void renumbers_designation_after_reorder()
    {
        using LIST_SCENARIO scenario = new("a.txt", "b.txt", "c.txt", "d.txt", "e.txt");
        scenario.list.begin_designation([scenario.id("a.txt"), scenario.id("b.txt")]);
        string before = scenario.list.number_text;

        scenario.engine.items_sort(SORT_KEY.name_natural, true);
        scenario.load();

        Assert.AreEqual("1, 2", before);
        Assert.AreEqual("4, 5", scenario.list.number_text);
    }

    private static string[] checked_names(ITEM_LIST_VIEW_MODEL list)
    {
        return list.rows.Where(row => row.is_checked).Select(row => row.name).ToArray();
    }
}
