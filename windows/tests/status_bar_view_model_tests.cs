using System;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace limenamer;

[TestClass]
public sealed class STATUS_BAR_VIEW_MODEL_TESTS
{
    [TestMethod]
    public void posts_messages_with_time()
    {
        using LIST_SCENARIO scenario = new();
        DateTime now = new(2026, 11, 9, 15, 42, 10);
        STATUS_BAR_VIEW_MODEL status = new(scenario.list, () => now);

        status.post("8번 항목을 직접 수정했습니다.");

        Assert.AreEqual("8번 항목을 직접 수정했습니다.", status.message);
        Assert.AreEqual("15:42", status.time_text);
    }

    [TestMethod]
    public void toggles_filter_from_problem_counts()
    {
        using LIST_SCENARIO scenario = STATUS_SCENARIO.create();
        STATUS_BAR_VIEW_MODEL status = new(scenario.list, () => DateTime.Now);

        Assert.IsTrue(status.has_errors && status.has_conflicts && status.has_warnings);
        Assert.AreEqual("오류 1", status.error_text);
        status.toggle_error.Execute(null);
        Assert.AreEqual(VIEW_FILTER.error, scenario.list.filter);
        Assert.AreEqual(VIEW_FILTER.error, status.active_filter);
        status.toggle_conflict.Execute(null);
        Assert.AreEqual(VIEW_FILTER.conflict, scenario.list.filter);
        status.toggle_conflict.Execute(null);
        Assert.AreEqual(VIEW_FILTER.all, scenario.list.filter);
    }

    [TestMethod]
    public void shows_progress_after_delay()
    {
        using LIST_SCENARIO scenario = new();
        DateTime now = new(2026, 11, 9, 15, 42, 0);
        STATUS_BAR_VIEW_MODEL status = new(scenario.list, () => now);
        PROGRESS progress = new();

        status.begin_progress(progress, "이름 바꾸는 중");
        progress.set(420, 1000);
        now += TimeSpan.FromSeconds(0.2);
        status.poll_progress();
        bool early = status.progress_visible;
        now += TimeSpan.FromSeconds(0.11);
        status.poll_progress();

        Assert.IsFalse(early);
        Assert.IsTrue(status.progress_visible);
        Assert.AreEqual("이름 바꾸는 중", status.progress_label);
        Assert.AreEqual("420 / 1,000", status.progress_text);
        Assert.AreEqual(0.42, status.progress_value, 1e-9);

        status.end_progress();

        Assert.IsFalse(status.progress_visible);
        Assert.AreEqual("", status.progress_text);
    }
}
