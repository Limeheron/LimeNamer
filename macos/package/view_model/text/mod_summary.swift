public enum MOD_SUMMARY
{
    public static func describe(_ mod: MOD, direct_index: UInt32, target_count: UInt32)
        -> String
    {
        var summary: String = mod.direct ? direct_edit(mod, direct_index) : body(mod)
        if mod.scope == .items && !mod.direct {
            summary += JOSA_FORMATTER.format(STRINGS.summary_scope, target_count)
        }
        return summary
    }

    private static func direct_edit(_ mod: MOD, _ direct_index: UInt32) -> String
    {
        let value: String = value_text(mod)
        if direct_index == 0 {
            return JOSA_FORMATTER.format(STRINGS.summary_direct_unlisted, value)
        }
        return JOSA_FORMATTER.format(STRINGS.summary_direct, direct_index, value)
    }

    private static func body(_ mod: MOD) -> String
    {
        let part: String = part_name(mod.part)
        let position: POSITION = mod.position
        switch mod.kind {
        case .insert:
            return JOSA_FORMATTER.format(STRINGS.summary_insert, place(part, position),
                value_text(mod))
        case .delete:
            return delete_body(part, position)
        case .replace:
            return JOSA_FORMATTER.format(STRINGS.summary_replace,
                target_phrase(part, position).text, value_text(mod))
        case .format:
            return format_body(mod, target_phrase(part, position).text)
        }
    }

    private static func delete_body(_ part: String, _ position: POSITION) -> String
    {
        let target: (text: String, plain: Bool) = position.type == .all
            ? (JOSA_FORMATTER.format(STRINGS.target_whole, part), true)
            : target_phrase(part, position)
        let template: String = target.plain ? STRINGS.summary_delete_plain : STRINGS.summary_delete
        return JOSA_FORMATTER.format(template, target.text)
    }

    private static func format_body(_ mod: MOD, _ target: String) -> String
    {
        let settings: FORMAT_SETTINGS = mod.format ?? FORMAT_SETTINGS()
        let how: String
        switch settings.type {
        case .pad_digits:
            return JOSA_FORMATTER.format(STRINGS.summary_pad_digits, target, settings.digits ?? 1)
        case .upper:
            how = STRINGS.format_upper
        case .lower:
            how = STRINGS.format_lower
        case .title:
            how = STRINGS.format_title
        case .sentence:
            how = STRINGS.format_sentence
        }
        return JOSA_FORMATTER.format(STRINGS.summary_format_case, target, how)
    }

    private static func place(_ part: String, _ position: POSITION) -> String
    {
        let n: UInt32 = position.n ?? 1
        switch position.type {
        case .start:
            return JOSA_FORMATTER.format(STRINGS.place_start, part)
        case .end:
            return JOSA_FORMATTER.format(STRINGS.place_end, part)
        case .after_nth:
            return JOSA_FORMATTER.format(STRINGS.place_after_nth, part, n)
        case .before_nth_from_end:
            return JOSA_FORMATTER.format(STRINGS.place_before_nth_from_end, part, n)
        case .before_text:
            return JOSA_FORMATTER.format(STRINGS.place_before_text, part,
                occurrence_word(position), quote(position.text))
        default:
            return JOSA_FORMATTER.format(STRINGS.place_after_text, part,
                occurrence_word(position), quote(position.text))
        }
    }

    private static func target_phrase(_ part: String, _ position: POSITION)
        -> (text: String, plain: Bool)
    {
        let n: UInt32 = position.n ?? 1
        switch position.type {
        case .first_n:
            return (JOSA_FORMATTER.format(STRINGS.target_first_n, part, n), true)
        case .last_n:
            return (JOSA_FORMATTER.format(STRINGS.target_last_n, part, n), true)
        case .from_nth:
            return (JOSA_FORMATTER.format(STRINGS.target_from_nth, part, n, position.count ?? 1),
                true)
        case .text:
            return (JOSA_FORMATTER.format(STRINGS.target_text, part, occurrence_word(position),
                quote(position.text)), false)
        case .between:
            return between_phrase(part, position)
        default:
            return (part, false)
        }
    }

    private static func between_phrase(_ part: String, _ position: POSITION)
        -> (text: String, plain: Bool)
    {
        let from_beginning: Bool = position.from_beginning ?? false
        let to_end: Bool = position.to_end ?? false
        let including: Bool = position.include_delimiters ?? false
        let prefix: String = (position.occurrence ?? .first) == .first
            ? ""
            : occurrence_word(position) + " "
        let start: String = quote(position.start_text)
        let end: String = quote(position.end_text)
        if from_beginning && to_end {
            return (JOSA_FORMATTER.format(STRINGS.target_whole, part), true)
        }
        if from_beginning {
            let template: String = including
                ? STRINGS.target_from_beginning_including
                : STRINGS.target_from_beginning
            return (JOSA_FORMATTER.format(template, part, prefix, end), true)
        }
        if to_end {
            let template: String = including
                ? STRINGS.target_to_end_including
                : STRINGS.target_to_end
            return (JOSA_FORMATTER.format(template, part, prefix, start), true)
        }
        if including {
            return (JOSA_FORMATTER.format(STRINGS.target_between_including, part, prefix, start,
                end), true)
        }
        return (JOSA_FORMATTER.format(STRINGS.target_between, part, prefix, start, end), false)
    }

    private static func occurrence_word(_ position: POSITION) -> String
    {
        let n: UInt32 = position.occurrence_n ?? 1
        switch position.occurrence ?? .first {
        case .first:
            return STRINGS.occurrence_first
        case .last:
            return STRINGS.occurrence_last
        case .nth:
            return JOSA_FORMATTER.format(STRINGS.occurrence_nth, n)
        case .nth_from_end:
            return JOSA_FORMATTER.format(STRINGS.occurrence_nth_from_end, n)
        case .all:
            return STRINGS.occurrence_all
        }
    }

    private static func value_text(_ mod: MOD) -> String
    {
        var text: String = ""
        for piece in mod.value ?? [] {
            if let token = piece.token {
                text += "{" + token_name(token) + "}"
            } else {
                text += piece.text ?? ""
            }
        }
        return quote(text)
    }

    private static func part_name(_ part: PART) -> String
    {
        switch part {
        case .name:
            return STRINGS.part_name
        case .ext:
            return STRINGS.part_ext
        case .full:
            return STRINGS.part_full
        }
    }

    private static func token_name(_ token: TOKEN_KIND) -> String
    {
        switch token {
        case .number:
            return STRINGS.token_number
        case .original_name:
            return STRINGS.token_original_name
        case .modified_date:
            return STRINGS.token_modified_date
        case .created_date:
            return STRINGS.token_created_date
        case .size:
            return STRINGS.token_size
        case .parent_folder:
            return STRINGS.token_parent_folder
        }
    }

    private static func quote(_ text: String?) -> String
    {
        return "\"" + (text ?? "") + "\""
    }
}
