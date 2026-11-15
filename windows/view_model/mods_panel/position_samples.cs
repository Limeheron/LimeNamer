using System.Collections.Generic;

namespace limenamer;

internal sealed class POSITION_SAMPLES
{
    private const string INSERT_VALUE = "2024_";
    private const string REPLACE_VALUE = "제주_";
    private const string BEFORE_ANCHOR = "0001";
    private const string AFTER_ANCHOR = "IMG";
    private const string RANGE_TEXT = "IMG_";
    private const string BETWEEN_START = "IMG";

    private static readonly POSITION_TYPE[] INSERT_TYPES = [
        POSITION_TYPE.start, POSITION_TYPE.end,
        POSITION_TYPE.after_nth, POSITION_TYPE.before_nth_from_end,
        POSITION_TYPE.before_text, POSITION_TYPE.after_text,
    ];
    private static readonly POSITION_TYPE[] RANGE_TYPES = [
        POSITION_TYPE.all, POSITION_TYPE.from_nth,
        POSITION_TYPE.first_n, POSITION_TYPE.last_n,
        POSITION_TYPE.text, POSITION_TYPE.between,
    ];

    private readonly ENGINE engine_;
    private readonly Dictionary<MOD_KIND, IReadOnlyList<POSITION_CHOICE>> cache_ = new();

    public POSITION_SAMPLES(ENGINE engine)
    {
        engine_ = engine;
    }

    public static POSITION_TYPE default_type(MOD_KIND kind)
    {
        return kind == MOD_KIND.insert ? POSITION_TYPE.start : POSITION_TYPE.all;
    }

    public static string label(POSITION_TYPE type)
    {
        return type switch {
            POSITION_TYPE.start => STRINGS.position_start,
            POSITION_TYPE.end => STRINGS.position_end,
            POSITION_TYPE.after_nth => STRINGS.position_short_after_nth,
            POSITION_TYPE.before_nth_from_end => STRINGS.position_short_before_nth_from_end,
            POSITION_TYPE.before_text => STRINGS.position_short_before_text,
            POSITION_TYPE.after_text => STRINGS.position_short_after_text,
            POSITION_TYPE.all => STRINGS.position_all,
            POSITION_TYPE.from_nth => STRINGS.position_short_from_nth,
            POSITION_TYPE.first_n => STRINGS.position_first_n,
            POSITION_TYPE.last_n => STRINGS.position_last_n,
            POSITION_TYPE.text => STRINGS.position_text,
            _ => STRINGS.position_between,
        };
    }

    public IReadOnlyList<POSITION_CHOICE> choices(MOD_KIND kind)
    {
        if (cache_.TryGetValue(kind, out IReadOnlyList<POSITION_CHOICE>? found)) {
            return found;
        }
        List<POSITION_CHOICE> result = new();
        foreach (POSITION_TYPE type in kind == MOD_KIND.insert ? INSERT_TYPES : RANGE_TYPES) {
            result.Add(make(kind, type));
        }
        cache_[kind] = result;
        return result;
    }

    private POSITION_CHOICE make(MOD_KIND kind, POSITION_TYPE type)
    {
        string description = STRINGS.ResourceManager.GetString($"position_desc_{type}",
            STRINGS.Culture) ?? "";
        if (kind == MOD_KIND.insert) {
            (string head, string mark, string tail) inserted = parts(sample(kind, type), true);
            List<SAMPLE_PIECE> after = pieces(
                (inserted.head, SAMPLE_STYLE.plain), (inserted.mark, SAMPLE_STYLE.mark),
                (inserted.tail, SAMPLE_STYLE.plain));
            return new POSITION_CHOICE(type, label(type), description,
                pieces((inserted.head + inserted.tail, SAMPLE_STYLE.plain)), after,
                insert_tile(type, inserted.head, inserted.tail));
        }
        (string head, string mark, string tail) range = parts(sample(MOD_KIND.delete, type),
            false);
        List<SAMPLE_PIECE> tile = range_tile(type, range);
        if (kind == MOD_KIND.format) {
            return new POSITION_CHOICE(type, label(type), description, tile, [], tile);
        }
        List<SAMPLE_PIECE> before = pieces((range.head, SAMPLE_STYLE.plain),
            (range.mark, SAMPLE_STYLE.removed), (range.tail, SAMPLE_STYLE.plain));
        List<SAMPLE_PIECE> changed = kind == MOD_KIND.replace
            ? pieces((range.head, SAMPLE_STYLE.plain), (REPLACE_VALUE, SAMPLE_STYLE.mark),
                (range.tail, SAMPLE_STYLE.plain))
            : pieces((range.head + range.tail, SAMPLE_STYLE.plain));
        return new POSITION_CHOICE(type, label(type), description, before, changed, tile);
    }

    private EXAMPLE sample(MOD_KIND kind, POSITION_TYPE type)
    {
        MOD mod = new() {
            kind = kind,
            part = PART.name,
            position = position(type),
            value = kind == MOD_KIND.insert ? [new VALUE_PIECE { text = INSERT_VALUE }] : null,
        };
        return engine_.mod_sample(MOD_JSON.write(mod));
    }

    private static POSITION position(POSITION_TYPE type)
    {
        return type switch {
            POSITION_TYPE.after_nth => new POSITION { type = type, n = 3 },
            POSITION_TYPE.before_nth_from_end => new POSITION { type = type, n = 4 },
            POSITION_TYPE.before_text => new POSITION { type = type, text = BEFORE_ANCHOR },
            POSITION_TYPE.after_text => new POSITION { type = type, text = AFTER_ANCHOR },
            POSITION_TYPE.first_n or POSITION_TYPE.last_n => new POSITION { type = type, n = 4 },
            POSITION_TYPE.from_nth => new POSITION { type = type, n = 2, count = 3 },
            POSITION_TYPE.text => new POSITION { type = type, text = RANGE_TEXT },
            POSITION_TYPE.between => new POSITION {
                type = type, start_text = BETWEEN_START, to_end = true,
            },
            _ => new POSITION { type = type },
        };
    }

    private static (string head, string mark, string tail) parts(EXAMPLE example, bool after)
    {
        return ITEM_ROW.split(after ? example.after : example.before, example.prefix_len,
            example.suffix_len);
    }

    private static List<SAMPLE_PIECE> insert_tile(POSITION_TYPE type, string head, string tail)
    {
        if (type == POSITION_TYPE.after_text && head.EndsWith(AFTER_ANCHOR)) {
            return pieces((head[..^AFTER_ANCHOR.Length], SAMPLE_STYLE.plain),
                (AFTER_ANCHOR, SAMPLE_STYLE.anchor), ("", SAMPLE_STYLE.caret),
                (tail, SAMPLE_STYLE.plain));
        }
        if (type == POSITION_TYPE.before_text && tail.StartsWith(BEFORE_ANCHOR)) {
            return pieces((head, SAMPLE_STYLE.plain), ("", SAMPLE_STYLE.caret),
                (BEFORE_ANCHOR, SAMPLE_STYLE.anchor), (tail[BEFORE_ANCHOR.Length..],
                    SAMPLE_STYLE.plain));
        }
        return pieces((head, SAMPLE_STYLE.plain), ("", SAMPLE_STYLE.caret),
            (tail, SAMPLE_STYLE.plain));
    }

    private static List<SAMPLE_PIECE> range_tile(POSITION_TYPE type,
        (string head, string mark, string tail) range)
    {
        if (type == POSITION_TYPE.text) {
            return pieces((range.head, SAMPLE_STYLE.plain), (range.mark, SAMPLE_STYLE.mark_anchor),
                (range.tail, SAMPLE_STYLE.plain));
        }
        if (type == POSITION_TYPE.between && range.head.EndsWith(BETWEEN_START)) {
            return pieces((range.head[..^BETWEEN_START.Length], SAMPLE_STYLE.plain),
                (BETWEEN_START, SAMPLE_STYLE.anchor), (range.mark, SAMPLE_STYLE.mark),
                (range.tail, SAMPLE_STYLE.plain));
        }
        return pieces((range.head, SAMPLE_STYLE.plain), (range.mark, SAMPLE_STYLE.mark),
            (range.tail, SAMPLE_STYLE.plain));
    }

    private static List<SAMPLE_PIECE> pieces(params (string text, SAMPLE_STYLE style)[] parts)
    {
        List<SAMPLE_PIECE> result = new();
        foreach ((string text, SAMPLE_STYLE style) in parts) {
            if (text.Length > 0 || style == SAMPLE_STYLE.caret) {
                result.Add(new SAMPLE_PIECE(text, style));
            }
        }
        return result;
    }
}
