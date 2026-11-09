using System;
using System.ComponentModel;
using System.Globalization;

namespace limenamer;

public sealed class STATUS_BAR_VIEW_MODEL : OBSERVABLE
{
    private static readonly TimeSpan PROGRESS_DELAY = TimeSpan.FromSeconds(0.3);

    private readonly ITEM_LIST_VIEW_MODEL list_;
    private readonly Func<DateTime> now_;
    private string message_ = "";
    private string time_text_ = "";
    private PROGRESS? progress_;
    private DateTime progress_start_;
    private string progress_label_ = "";
    private bool progress_visible_;
    private string progress_text_ = "";
    private double progress_value_;

    public STATUS_BAR_VIEW_MODEL(ITEM_LIST_VIEW_MODEL list, Func<DateTime> now)
    {
        list_ = list;
        now_ = now;
        list_.PropertyChanged += on_list_changed;
        toggle_error = new COMMAND(_ => toggle(VIEW_FILTER.error));
        toggle_conflict = new COMMAND(_ => toggle(VIEW_FILTER.conflict));
        toggle_warning = new COMMAND(_ => toggle(VIEW_FILTER.warning));
    }

    public COMMAND toggle_error { get; }
    public COMMAND toggle_conflict { get; }
    public COMMAND toggle_warning { get; }

    public string message
    {
        get { return message_; }
        private set { assign(ref message_, value); }
    }

    public string time_text
    {
        get { return time_text_; }
        private set { assign(ref time_text_, value); }
    }

    public string error_text
    {
        get { return JOSA_FORMATTER.format(STRINGS.pill_error, list_.count_error); }
    }

    public string conflict_text
    {
        get { return JOSA_FORMATTER.format(STRINGS.pill_conflict, list_.count_conflict); }
    }

    public string warning_text
    {
        get { return JOSA_FORMATTER.format(STRINGS.pill_warning, list_.count_warning); }
    }

    public bool has_errors
    {
        get { return list_.count_error > 0; }
    }

    public bool has_conflicts
    {
        get { return list_.count_conflict > 0; }
    }

    public bool has_warnings
    {
        get { return list_.count_warning > 0; }
    }

    public VIEW_FILTER active_filter
    {
        get { return list_.filter; }
    }

    public string progress_label
    {
        get { return progress_label_; }
        private set { assign(ref progress_label_, value); }
    }

    public bool progress_visible
    {
        get { return progress_visible_; }
        private set { assign(ref progress_visible_, value); }
    }

    public string progress_text
    {
        get { return progress_text_; }
        private set { assign(ref progress_text_, value); }
    }

    public double progress_value
    {
        get { return progress_value_; }
        private set { assign(ref progress_value_, value); }
    }

    public void post(string text)
    {
        message = text;
        time_text = now_().ToString("HH:mm", CultureInfo.InvariantCulture);
    }

    public void begin_progress(PROGRESS progress, string label)
    {
        progress_ = progress;
        progress_start_ = now_();
        progress_label = label;
        progress_visible = false;
    }

    public void poll_progress()
    {
        if (progress_ == null) {
            return;
        }
        (uint done, uint total) = progress_.read();
        progress_text = JOSA_FORMATTER.format(STRINGS.progress_count,
            done.ToString("#,0", CultureInfo.InvariantCulture),
            total.ToString("#,0", CultureInfo.InvariantCulture));
        progress_value = total == 0 ? 0 : (double)done / total;
        if (now_() - progress_start_ >= PROGRESS_DELAY) {
            progress_visible = true;
        }
    }

    public void end_progress()
    {
        progress_ = null;
        progress_visible = false;
        progress_label = "";
        progress_text = "";
        progress_value = 0;
    }

    private void toggle(VIEW_FILTER filter)
    {
        list_.filter = list_.filter == filter ? VIEW_FILTER.all : filter;
    }

    private void on_list_changed(object? sender, PropertyChangedEventArgs args)
    {
        switch (args.PropertyName) {
        case nameof(ITEM_LIST_VIEW_MODEL.count_error):
            notify(nameof(error_text));
            notify(nameof(has_errors));
            break;
        case nameof(ITEM_LIST_VIEW_MODEL.count_conflict):
            notify(nameof(conflict_text));
            notify(nameof(has_conflicts));
            break;
        case nameof(ITEM_LIST_VIEW_MODEL.count_warning):
            notify(nameof(warning_text));
            notify(nameof(has_warnings));
            break;
        case nameof(ITEM_LIST_VIEW_MODEL.filter):
            notify(nameof(active_filter));
            break;
        }
    }
}
