using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;

namespace limenamer;

public sealed class MOD_EDITOR_VIEW_MODEL : OBSERVABLE
{
    private const int NUMBER_LIMIT = 1_000_000_000;

    private static readonly CHOICE<OCCURRENCE>[] OCCURRENCES = [
        new(OCCURRENCE.first, STRINGS.occurrence_choice_first),
        new(OCCURRENCE.last, STRINGS.occurrence_choice_last),
        new(OCCURRENCE.nth, STRINGS.occurrence_choice_nth),
        new(OCCURRENCE.nth_from_end, STRINGS.occurrence_choice_nth_from_end),
        new(OCCURRENCE.all, STRINGS.occurrence_choice_all),
    ];
    private static readonly string[] DERIVED = [
        nameof(position_tiles), nameof(occurrence_choices), nameof(shows_n),
        nameof(shows_count), nameof(shows_text), nameof(shows_occurrence),
        nameof(shows_occurrence_n), nameof(shows_between), nameof(shows_start_text),
        nameof(shows_end_text), nameof(shows_value), nameof(shows_format), nameof(shows_digits),
        nameof(shows_value_or_format),
        nameof(uses_number), nameof(uses_modified_date), nameof(uses_created_date),
        nameof(uses_size), nameof(shows_modified_date_custom), nameof(shows_created_date_custom),
    ];

    private static readonly string[] EXAMPLE_PARTS = [
        nameof(example_before_head), nameof(example_before_mark), nameof(example_before_tail),
        nameof(example_after_head), nameof(example_after_mark), nameof(example_after_tail),
    ];

    private readonly ENGINE engine_;
    private readonly ITEM_LIST_VIEW_MODEL list_;
    private readonly POSITION_SAMPLES samples_;
    private bool loading_;
    private bool active_;
    private bool tried_;
    private string original_json_ = "";
    private ulong editing_id_;
    private MOD_KIND kind_;
    private PART part_;
    private POSITION_TYPE position_type_;
    private uint n_ = 1;
    private uint count_ = 1;
    private string find_text_ = "";
    private bool case_sensitive_;
    private OCCURRENCE occurrence_;
    private uint occurrence_n_ = 1;
    private bool from_beginning_;
    private string start_text_ = "";
    private bool to_end_;
    private string end_text_ = "";
    private bool include_delimiters_;
    private IReadOnlyList<VALUE_PIECE> value_ = [];
    private int number_start_ = 1;
    private int number_step_ = 1;
    private uint number_digits_ = 3;
    private string modified_date_choice_ = "yyyyMMdd";
    private string modified_date_custom_ = "";
    private string created_date_choice_ = "yyyyMMdd";
    private string created_date_custom_ = "";
    private SIZE_UNIT size_unit_ = SIZE_UNIT.auto;
    private FORMAT_TYPE format_type_ = FORMAT_TYPE.upper;
    private uint format_digits_ = 3;
    private SCOPE scope_;
    private HashSet<ulong> last_targets_ = new();
    private string summary_ = "";
    private bool has_example_;
    private string example_index_text_ = "";
    private string example_state_text_ = "";
    private (string head, string mark, string tail) example_before_ = ("", "", "");
    private (string head, string mark, string tail) example_after_ = ("", "", "");
    private Dictionary<MOD_FIELD, string> problems_ = new();

    public MOD_EDITOR_VIEW_MODEL(ENGINE engine, ITEM_LIST_VIEW_MODEL list)
    {
        engine_ = engine;
        list_ = list;
        samples_ = new POSITION_SAMPLES(engine);
        list_.PropertyChanged += on_list_changed;
    }

    public event Action<string>? notice;
    public event Action<MOD_FIELD>? focus_requested;

    public static IReadOnlyList<CHOICE<PART>> part_choices { get; } = [
        new(PART.name, STRINGS.part_name),
        new(PART.ext, STRINGS.part_ext),
        new(PART.full, STRINGS.part_full),
    ];

    public static IReadOnlyList<CHOICE<SCOPE>> scope_choices { get; } = [
        new(SCOPE.all, STRINGS.scope_all),
        new(SCOPE.items, STRINGS.scope_items),
    ];

    public static IReadOnlyList<CHOICE<FORMAT_TYPE>> format_choices { get; } = [
        new(FORMAT_TYPE.upper, STRINGS.format_choice_upper),
        new(FORMAT_TYPE.lower, STRINGS.format_choice_lower),
        new(FORMAT_TYPE.title, STRINGS.format_choice_title),
        new(FORMAT_TYPE.sentence, STRINGS.format_choice_sentence),
        new(FORMAT_TYPE.pad_digits, STRINGS.format_choice_pad_digits),
    ];

    public static IReadOnlyList<CHOICE<SIZE_UNIT>> size_unit_choices { get; } = [
        new(SIZE_UNIT.auto, STRINGS.size_unit_auto),
        new(SIZE_UNIT.b, STRINGS.size_unit_b),
        new(SIZE_UNIT.kb, STRINGS.size_unit_kb),
        new(SIZE_UNIT.mb, STRINGS.size_unit_mb),
        new(SIZE_UNIT.gb, STRINGS.size_unit_gb),
    ];

    public static IReadOnlyList<CHOICE<string>> date_format_choices { get; } = [
        new("yyyyMMdd", "yyyyMMdd"),
        new("yyyy-MM-dd", "yyyy-MM-dd"),
        new("yyyy_MM_dd", "yyyy_MM_dd"),
        new("yyMMdd", "yyMMdd"),
        new("yyyyMMdd_HHmmss", "yyyyMMdd_HHmmss"),
        new("", STRINGS.date_custom),
    ];

    public bool is_new
    {
        get { return editing_id_ == 0; }
    }

    public bool is_active
    {
        get { return active_; }
    }

    public ulong editing_id
    {
        get { return editing_id_; }
    }

    public MOD_KIND kind
    {
        get { return kind_; }
    }

    public PART part
    {
        get { return part_; }
        set { change(ref part_, value); }
    }

    public POSITION_TYPE position_type
    {
        get { return position_type_; }
        set {
            if (value == POSITION_TYPE.between && occurrence_ == OCCURRENCE.all) {
                assign(ref occurrence_, OCCURRENCE.first, nameof(occurrence));
            }
            change(ref position_type_, value);
        }
    }

    public uint n
    {
        get { return n_; }
        set { change(ref n_, Math.Max(1u, value)); }
    }

    public uint count
    {
        get { return count_; }
        set { change(ref count_, Math.Max(1u, value)); }
    }

    public string find_text
    {
        get { return find_text_; }
        set { change(ref find_text_, value); }
    }

    public bool case_sensitive
    {
        get { return case_sensitive_; }
        set { change(ref case_sensitive_, value); }
    }

    public OCCURRENCE occurrence
    {
        get { return occurrence_; }
        set { change(ref occurrence_, value); }
    }

    public uint occurrence_n
    {
        get { return occurrence_n_; }
        set { change(ref occurrence_n_, Math.Max(1u, value)); }
    }

    public bool from_beginning
    {
        get { return from_beginning_; }
        set { change(ref from_beginning_, value); }
    }

    public string start_text
    {
        get { return start_text_; }
        set { change(ref start_text_, value); }
    }

    public bool to_end
    {
        get { return to_end_; }
        set { change(ref to_end_, value); }
    }

    public string end_text
    {
        get { return end_text_; }
        set { change(ref end_text_, value); }
    }

    public bool include_delimiters
    {
        get { return include_delimiters_; }
        set { change(ref include_delimiters_, value); }
    }

    public IReadOnlyList<VALUE_PIECE> value
    {
        get { return value_; }
        set { change(ref value_, normalize(value)); }
    }

    public int number_start
    {
        get { return number_start_; }
        set { change(ref number_start_, Math.Clamp(value, -NUMBER_LIMIT, NUMBER_LIMIT)); }
    }

    public int number_step
    {
        get { return number_step_; }
        set { change(ref number_step_, Math.Clamp(value, -NUMBER_LIMIT, NUMBER_LIMIT)); }
    }

    public uint number_digits
    {
        get { return number_digits_; }
        set { change(ref number_digits_, Math.Clamp(value, 1u, 32u)); }
    }

    public string modified_date_choice
    {
        get { return modified_date_choice_; }
        set { change(ref modified_date_choice_, value); }
    }

    public string modified_date_custom
    {
        get { return modified_date_custom_; }
        set { change(ref modified_date_custom_, value); }
    }

    public string created_date_choice
    {
        get { return created_date_choice_; }
        set { change(ref created_date_choice_, value); }
    }

    public string created_date_custom
    {
        get { return created_date_custom_; }
        set { change(ref created_date_custom_, value); }
    }

    public SIZE_UNIT size_unit
    {
        get { return size_unit_; }
        set { change(ref size_unit_, value); }
    }

    public FORMAT_TYPE format_type
    {
        get { return format_type_; }
        set { change(ref format_type_, value); }
    }

    public uint format_digits
    {
        get { return format_digits_; }
        set { change(ref format_digits_, Math.Clamp(value, 1u, 32u)); }
    }

    public SCOPE scope
    {
        get { return scope_; }
        set {
            if (value == scope_) {
                return;
            }
            if (value == SCOPE.items) {
                list_.begin_designation(last_targets_);
            } else {
                last_targets_ = list_.checked_ids.ToHashSet();
                list_.end_designation();
            }
            change(ref scope_, value);
        }
    }

    public IReadOnlyList<POSITION_CHOICE> position_tiles
    {
        get { return samples_.choices(kind_); }
    }

    public IReadOnlyList<CHOICE<OCCURRENCE>> occurrence_choices
    {
        get { return position_type_ == POSITION_TYPE.between ? OCCURRENCES[..4] : OCCURRENCES; }
    }

    public bool shows_n
    {
        get {
            return position_type_ is POSITION_TYPE.after_nth or POSITION_TYPE.before_nth_from_end
                or POSITION_TYPE.first_n or POSITION_TYPE.last_n or POSITION_TYPE.from_nth;
        }
    }

    public bool shows_count
    {
        get { return position_type_ == POSITION_TYPE.from_nth; }
    }

    public bool shows_text
    {
        get {
            return position_type_ is POSITION_TYPE.before_text or POSITION_TYPE.after_text
                or POSITION_TYPE.text;
        }
    }

    public bool shows_occurrence
    {
        get { return shows_text || shows_between; }
    }

    public bool shows_occurrence_n
    {
        get {
            return shows_occurrence
                && occurrence_ is OCCURRENCE.nth or OCCURRENCE.nth_from_end;
        }
    }

    public bool shows_between
    {
        get { return position_type_ == POSITION_TYPE.between; }
    }

    public bool shows_start_text
    {
        get { return shows_between && !from_beginning_; }
    }

    public bool shows_end_text
    {
        get { return shows_between && !to_end_; }
    }

    public string value_label
    {
        get {
            return kind_ == MOD_KIND.insert
                ? STRINGS.field_value_insert
                : STRINGS.field_value_replace;
        }
    }

    public bool shows_value
    {
        get { return kind_ is MOD_KIND.insert or MOD_KIND.replace; }
    }

    public bool shows_format
    {
        get { return kind_ == MOD_KIND.format; }
    }

    public bool shows_value_or_format
    {
        get { return shows_value || shows_format; }
    }

    public bool shows_digits
    {
        get { return shows_format && format_type_ == FORMAT_TYPE.pad_digits; }
    }

    public bool uses_number
    {
        get { return uses(TOKEN_KIND.number); }
    }

    public bool uses_modified_date
    {
        get { return uses(TOKEN_KIND.modified_date); }
    }

    public bool uses_created_date
    {
        get { return uses(TOKEN_KIND.created_date); }
    }

    public bool uses_size
    {
        get { return uses(TOKEN_KIND.size); }
    }

    public bool shows_modified_date_custom
    {
        get { return uses_modified_date && modified_date_choice_ == ""; }
    }

    public bool shows_created_date_custom
    {
        get { return uses_created_date && created_date_choice_ == ""; }
    }

    public string summary
    {
        get { return summary_; }
        private set { assign(ref summary_, value); }
    }

    public bool has_example
    {
        get { return has_example_; }
        private set { assign(ref has_example_, value); }
    }

    public string example_index_text
    {
        get { return example_index_text_; }
        private set { assign(ref example_index_text_, value); }
    }

    public string example_state_text
    {
        get { return example_state_text_; }
        private set { assign(ref example_state_text_, value); }
    }

    public string example_before_head
    {
        get { return example_before_.head; }
    }

    public string example_before_mark
    {
        get { return example_before_.mark; }
    }

    public string example_before_tail
    {
        get { return example_before_.tail; }
    }

    public string example_after_head
    {
        get { return example_after_.head; }
    }

    public string example_after_mark
    {
        get { return example_after_.mark; }
    }

    public string example_after_tail
    {
        get { return example_after_.tail; }
    }

    public string targets_problem
    {
        get { return problem(MOD_FIELD.targets); }
    }

    public string find_text_problem
    {
        get { return problem(MOD_FIELD.find_text); }
    }

    public string start_text_problem
    {
        get { return problem(MOD_FIELD.start_text); }
    }

    public string end_text_problem
    {
        get { return problem(MOD_FIELD.end_text); }
    }

    public string value_problem
    {
        get { return problem(MOD_FIELD.value); }
    }

    public string modified_date_format_problem
    {
        get { return problem(MOD_FIELD.modified_date_format); }
    }

    public string created_date_format_problem
    {
        get { return problem(MOD_FIELD.created_date_format); }
    }

    public static string part_tip(PART part)
    {
        return part switch {
            PART.name => STRINGS.part_tip_name,
            PART.ext => STRINGS.part_tip_ext,
            _ => STRINGS.part_tip_full,
        };
    }

    public IReadOnlyList<POSITION_CHOICE> position_choices(MOD_KIND kind)
    {
        return samples_.choices(kind);
    }

    public void begin_new(MOD_KIND kind, POSITION_TYPE position)
    {
        begin(fresh(kind, position), 0);
    }

    public void restart(MOD_KIND kind, POSITION_TYPE position)
    {
        string original = original_json_;
        begin(fresh(kind, position), editing_id_);
        original_json_ = original;
    }

    public void begin_edit(MOD mod)
    {
        begin(mod, mod.id ?? 0);
    }

    public bool apply()
    {
        tried_ = true;
        if (!check()) {
            List<MOD_FIELD> fields = problems_.Keys.Order().ToList();
            string text = JOSA_FORMATTER.format(STRINGS.apply_failed, problems_[fields[0]]);
            if (fields.Count > 1) {
                text += JOSA_FORMATTER.format(STRINGS.apply_failed_more, fields.Count);
            }
            notice?.Invoke(text);
            focus_requested?.Invoke(fields[0]);
            return false;
        }

        string json = MOD_JSON.write(build());
        if (editing_id_ == 0) {
            engine_.mod_add(json);
        } else {
            engine_.mod_update(editing_id_, json);
        }
        finish();
        return true;
    }

    public string cancel()
    {
        bool dirty = MOD_JSON.write(build()) != original_json_;
        bool was_new = is_new;
        finish();
        if (was_new) {
            return STRINGS.draft_discarded_new;
        }
        return dirty ? STRINGS.draft_discarded : "";
    }

    public void refresh()
    {
        summary = MOD_SUMMARY.describe(build(), 0, target_count());
        refresh_example();
        if (tried_) {
            try {
                check();
            } catch (ENGINE_EXCEPTION) {
                return;
            }
        }
    }

    private void begin(MOD mod, ulong id)
    {
        loading_ = true;
        editing_id_ = id;
        kind_ = mod.kind;
        part = mod.part;
        POSITION position = mod.position;
        position_type = position.type;
        n = position.n ?? 1;
        count = position.count ?? 1;
        find_text = position.text ?? "";
        case_sensitive = position.case_sensitive ?? false;
        occurrence = position.occurrence ?? OCCURRENCE.first;
        occurrence_n = position.occurrence_n ?? 1;
        from_beginning = position.from_beginning ?? false;
        start_text = position.start_text ?? "";
        to_end = position.to_end ?? false;
        end_text = position.end_text ?? "";
        include_delimiters = position.include_delimiters ?? false;
        value = mod.value ?? [];
        NUMBER_SETTINGS number = mod.tokens?.number ?? new NUMBER_SETTINGS { digits = 3 };
        number_start = number.start;
        number_step = number.step;
        number_digits = number.digits;
        (modified_date_choice, modified_date_custom) =
            split_date(mod.tokens?.modified_date?.format);
        (created_date_choice, created_date_custom) = split_date(mod.tokens?.created_date?.format);
        size_unit = mod.tokens?.size?.unit ?? SIZE_UNIT.auto;
        format_type = mod.format?.type ?? FORMAT_TYPE.upper;
        format_digits = mod.format?.digits ?? 3;
        last_targets_ = new HashSet<ulong>(mod.targets ?? []);
        list_.end_designation();
        scope_ = SCOPE.all;
        scope = mod.scope;
        tried_ = false;
        problems_ = new Dictionary<MOD_FIELD, string>();
        loading_ = false;
        active_ = true;
        original_json_ = MOD_JSON.write(build());
        notify(null);
        refresh();
    }

    private void finish()
    {
        active_ = false;
        list_.end_designation();
        tried_ = false;
        problems_ = new Dictionary<MOD_FIELD, string>();
        notify_problems();
    }

    private void change<T>(ref T field, T value, [CallerMemberName] string? name = null)
    {
        if (!assign(ref field, value, name) || loading_) {
            return;
        }
        foreach (string derived in DERIVED) {
            notify(derived);
        }
        refresh();
    }

    private void on_list_changed(object? sender, PropertyChangedEventArgs args)
    {
        if (!active_ || loading_ || scope_ != SCOPE.items) {
            return;
        }
        if (args.PropertyName is nameof(ITEM_LIST_VIEW_MODEL.checked_ids)
                or nameof(ITEM_LIST_VIEW_MODEL.number_error)) {
            refresh();
        }
    }

    private bool check()
    {
        Dictionary<MOD_FIELD, string> problems = new();
        foreach (PROBLEM item in engine_.mod_check(MOD_JSON.write(build()))) {
            problems[item.field] = MESSAGES.problem_text(item, kind_, false);
        }
        if (scope_ == SCOPE.items && list_.number_error != "") {
            problems[MOD_FIELD.targets] = list_.number_error;
        }
        problems_ = problems;
        notify_problems();
        return problems.Count == 0;
    }

    private void notify_problems()
    {
        notify(nameof(targets_problem));
        notify(nameof(find_text_problem));
        notify(nameof(start_text_problem));
        notify(nameof(end_text_problem));
        notify(nameof(value_problem));
        notify(nameof(modified_date_format_problem));
        notify(nameof(created_date_format_problem));
    }

    private string problem(MOD_FIELD field)
    {
        return problems_.TryGetValue(field, out string? text) ? text : "";
    }

    private void refresh_example()
    {
        EXAMPLE example;
        try {
            example = engine_.mod_example(MOD_JSON.write(build()), editing_id_);
        } catch (ENGINE_EXCEPTION) {
            has_example = false;
            return;
        }
        example_index_text = example.item_id == 0
            ? STRINGS.example_default
            : JOSA_FORMATTER.format(STRINGS.example_index, example.index);
        example_state_text = MESSAGES.example_state_text(example.state);
        example_before_ = ITEM_ROW.split(example.before, example.prefix_len, example.suffix_len);
        example_after_ = ITEM_ROW.split(example.after, example.prefix_len, example.suffix_len);
        foreach (string name in EXAMPLE_PARTS) {
            notify(name);
        }
        has_example = true;
    }

    private MOD build()
    {
        bool has_value = kind_ is MOD_KIND.insert or MOD_KIND.replace;
        return new MOD {
            kind = kind_,
            part = part_,
            position = new POSITION {
                type = position_type_,
                n = n_,
                count = count_,
                text = find_text_,
                case_sensitive = case_sensitive_,
                occurrence = occurrence_,
                occurrence_n = occurrence_n_,
                from_beginning = from_beginning_,
                start_text = start_text_,
                to_end = to_end_,
                end_text = end_text_,
                include_delimiters = include_delimiters_,
            },
            value = has_value ? value_ : null,
            tokens = has_value ? new TOKEN_SETTINGS {
                number = new NUMBER_SETTINGS {
                    start = number_start_,
                    step = number_step_,
                    digits = number_digits_,
                },
                modified_date = new DATE_SETTINGS {
                    format = date_format(modified_date_choice_, modified_date_custom_),
                },
                created_date = new DATE_SETTINGS {
                    format = date_format(created_date_choice_, created_date_custom_),
                },
                size = new SIZE_SETTINGS { unit = size_unit_ },
            } : null,
            format = kind_ == MOD_KIND.format ? new FORMAT_SETTINGS {
                type = format_type_,
                digits = format_type_ == FORMAT_TYPE.pad_digits ? format_digits_ : null,
            } : null,
            scope = scope_,
            targets = scope_ == SCOPE.items ? list_.live_checked_ids() : null,
        };
    }

    private static MOD fresh(MOD_KIND kind, POSITION_TYPE position)
    {
        return new MOD {
            kind = kind,
            part = PART.name,
            position = new POSITION { type = position },
        };
    }

    private uint target_count()
    {
        return scope_ == SCOPE.items ? (uint)list_.live_checked_ids().Count : 0;
    }

    private bool uses(TOKEN_KIND token)
    {
        return shows_value && value_.Any(piece => piece.token == token);
    }

    private static (string choice, string custom) split_date(string? format)
    {
        string text = format ?? "yyyyMMdd";
        if (date_format_choices.Any(choice => choice.value != "" && choice.value == text)) {
            return (text, "");
        }
        return ("", text);
    }

    private static string date_format(string choice, string custom)
    {
        return choice == "" ? custom : choice;
    }

    private static IReadOnlyList<VALUE_PIECE> normalize(IEnumerable<VALUE_PIECE> pieces)
    {
        List<VALUE_PIECE> result = new();
        foreach (VALUE_PIECE piece in pieces) {
            if (piece.token != null) {
                result.Add(new VALUE_PIECE { token = piece.token });
            } else if (!string.IsNullOrEmpty(piece.text)) {
                if (result.Count > 0 && result[^1].token == null) {
                    result[^1] = new VALUE_PIECE { text = result[^1].text + piece.text };
                } else {
                    result.Add(new VALUE_PIECE { text = piece.text });
                }
            }
        }
        return result;
    }
}
