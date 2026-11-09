using System.Text;

namespace limenamer;

public static class MOD_SUMMARY
{
    public static string describe(MOD mod, uint direct_index, uint target_count)
    {
        string summary = mod.direct ? direct_edit(mod, direct_index) : body(mod);
        if (mod.scope == SCOPE.items && !mod.direct) {
            summary += JOSA_FORMATTER.format(STRINGS.summary_scope, target_count);
        }
        return summary;
    }

    private static string direct_edit(MOD mod, uint direct_index)
    {
        string value = value_text(mod);
        if (direct_index == 0) {
            return JOSA_FORMATTER.format(STRINGS.summary_direct_unlisted, value);
        }
        return JOSA_FORMATTER.format(STRINGS.summary_direct, direct_index, value);
    }

    private static string body(MOD mod)
    {
        string part = part_name(mod.part);
        POSITION position = mod.position;
        switch (mod.kind) {
        case MOD_KIND.insert:
            return JOSA_FORMATTER.format(STRINGS.summary_insert, place(part, position),
                value_text(mod));
        case MOD_KIND.delete:
            return delete_body(part, position);
        case MOD_KIND.replace:
            return JOSA_FORMATTER.format(STRINGS.summary_replace,
                target_phrase(part, position).text, value_text(mod));
        default:
            return format_body(mod, target_phrase(part, position).text);
        }
    }

    private static string delete_body(string part, POSITION position)
    {
        (string text, bool plain) target = position.type == POSITION_TYPE.all
            ? (JOSA_FORMATTER.format(STRINGS.target_whole, part), true)
            : target_phrase(part, position);
        string template = target.plain ? STRINGS.summary_delete_plain : STRINGS.summary_delete;
        return JOSA_FORMATTER.format(template, target.text);
    }

    private static string format_body(MOD mod, string target)
    {
        FORMAT_SETTINGS settings = mod.format ?? new FORMAT_SETTINGS();
        if (settings.type == FORMAT_TYPE.pad_digits) {
            return JOSA_FORMATTER.format(STRINGS.summary_pad_digits, target, settings.digits ?? 1);
        }

        string how = settings.type switch {
            FORMAT_TYPE.upper => STRINGS.format_upper,
            FORMAT_TYPE.lower => STRINGS.format_lower,
            FORMAT_TYPE.title => STRINGS.format_title,
            _ => STRINGS.format_sentence,
        };
        return JOSA_FORMATTER.format(STRINGS.summary_format_case, target, how);
    }

    private static string place(string part, POSITION position)
    {
        uint n = position.n ?? 1;
        return position.type switch {
            POSITION_TYPE.start => JOSA_FORMATTER.format(STRINGS.place_start, part),
            POSITION_TYPE.end => JOSA_FORMATTER.format(STRINGS.place_end, part),
            POSITION_TYPE.after_nth => JOSA_FORMATTER.format(STRINGS.place_after_nth, part, n),
            POSITION_TYPE.before_nth_from_end =>
                JOSA_FORMATTER.format(STRINGS.place_before_nth_from_end, part, n),
            POSITION_TYPE.before_text => JOSA_FORMATTER.format(STRINGS.place_before_text, part,
                occurrence_word(position), quote(position.text)),
            _ => JOSA_FORMATTER.format(STRINGS.place_after_text, part, occurrence_word(position),
                quote(position.text)),
        };
    }

    private static (string text, bool plain) target_phrase(string part, POSITION position)
    {
        uint n = position.n ?? 1;
        switch (position.type) {
        case POSITION_TYPE.first_n:
            return (JOSA_FORMATTER.format(STRINGS.target_first_n, part, n), true);
        case POSITION_TYPE.last_n:
            return (JOSA_FORMATTER.format(STRINGS.target_last_n, part, n), true);
        case POSITION_TYPE.from_nth:
            return (JOSA_FORMATTER.format(STRINGS.target_from_nth, part, n, position.count ?? 1),
                true);
        case POSITION_TYPE.text:
            return (JOSA_FORMATTER.format(STRINGS.target_text, part, occurrence_word(position),
                quote(position.text)), false);
        case POSITION_TYPE.between:
            return between_phrase(part, position);
        default:
            return (part, false);
        }
    }

    private static (string text, bool plain) between_phrase(string part, POSITION position)
    {
        bool from_beginning = position.from_beginning ?? false;
        bool to_end = position.to_end ?? false;
        bool including = position.include_delimiters ?? false;
        string prefix = (position.occurrence ?? OCCURRENCE.first) == OCCURRENCE.first
            ? ""
            : occurrence_word(position) + " ";
        string start = quote(position.start_text);
        string end = quote(position.end_text);

        if (from_beginning && to_end) {
            return (JOSA_FORMATTER.format(STRINGS.target_whole, part), true);
        }
        if (from_beginning) {
            string template = including
                ? STRINGS.target_from_beginning_including
                : STRINGS.target_from_beginning;
            return (JOSA_FORMATTER.format(template, part, prefix, end), true);
        }
        if (to_end) {
            string template = including ? STRINGS.target_to_end_including : STRINGS.target_to_end;
            return (JOSA_FORMATTER.format(template, part, prefix, start), true);
        }
        if (including) {
            return (JOSA_FORMATTER.format(STRINGS.target_between_including, part, prefix, start,
                end), true);
        }
        return (JOSA_FORMATTER.format(STRINGS.target_between, part, prefix, start, end), false);
    }

    private static string occurrence_word(POSITION position)
    {
        uint n = position.occurrence_n ?? 1;
        return (position.occurrence ?? OCCURRENCE.first) switch {
            OCCURRENCE.first => STRINGS.occurrence_first,
            OCCURRENCE.last => STRINGS.occurrence_last,
            OCCURRENCE.nth => JOSA_FORMATTER.format(STRINGS.occurrence_nth, n),
            OCCURRENCE.nth_from_end => JOSA_FORMATTER.format(STRINGS.occurrence_nth_from_end, n),
            _ => STRINGS.occurrence_all,
        };
    }

    private static string value_text(MOD mod)
    {
        StringBuilder text = new();
        foreach (VALUE_PIECE piece in mod.value ?? []) {
            if (piece.token is TOKEN_KIND token) {
                text.Append('{').Append(token_name(token)).Append('}');
            } else {
                text.Append(piece.text);
            }
        }
        return quote(text.ToString());
    }

    private static string part_name(PART part)
    {
        return part switch {
            PART.name => STRINGS.part_name,
            PART.ext => STRINGS.part_ext,
            _ => STRINGS.part_full,
        };
    }

    private static string token_name(TOKEN_KIND token)
    {
        return token switch {
            TOKEN_KIND.number => STRINGS.token_number,
            TOKEN_KIND.original_name => STRINGS.token_original_name,
            TOKEN_KIND.modified_date => STRINGS.token_modified_date,
            TOKEN_KIND.created_date => STRINGS.token_created_date,
            TOKEN_KIND.size => STRINGS.token_size,
            _ => STRINGS.token_parent_folder,
        };
    }

    private static string quote(string? text)
    {
        return "\"" + text + "\"";
    }
}
