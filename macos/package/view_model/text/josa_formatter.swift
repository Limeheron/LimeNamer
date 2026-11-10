public enum JOSA_FORMATTER
{
    private static let RIEUL: Int = 8
    private static let DIGIT_FINALS: [Int] = [21, 8, 0, 16, 0, 0, 1, 8, 8, 0]

    public static func format(_ template: String, _ args: Any...) -> String
    {
        var result: String = ""
        var index: String.Index = template.startIndex
        while index < template.endIndex {
            let character: Character = template[index]
            let next: String.Index = template.index(after: index)
            if (character == "{" || character == "}") && next < template.endIndex
                    && template[next] == character {
                result.append(character)
                index = template.index(after: next)
            } else if character == "{", let end = template[next...].firstIndex(of: "}") {
                result += item(template[next..<end], args)
                index = template.index(after: end)
            } else {
                result.append(character)
                index = next
            }
        }
        return result
    }

    static func final_consonant(_ text: String) -> Int
    {
        guard let last = text.unicodeScalars.last(where: { !is_closing($0) }) else {
            return 0
        }
        let code: UInt32 = last.value
        if code >= 0xAC00 && code <= 0xD7A3 {
            return Int((code - 0xAC00) % 28)
        }
        if code >= 0x30 && code <= 0x39 {
            return DIGIT_FINALS[Int(code - 0x30)]
        }
        if last == "l" || last == "L" {
            return RIEUL
        }
        if last == "m" || last == "n" || last == "M" || last == "N" {
            return 4
        }
        return 0
    }

    private static func item(_ spec: Substring, _ args: [Any]) -> String
    {
        let parts: [Substring] = spec.split(separator: ":", maxSplits: 1,
            omittingEmptySubsequences: false)
        guard let position = Int(parts[0]), position >= 0, position < args.count else {
            return ""
        }
        let text: String = String(describing: args[position])
        let forms: [Substring] = parts.count == 2 ? parts[1].split(separator: "/",
            omittingEmptySubsequences: false) : []
        if forms.count != 2 {
            return text
        }
        let final: Int = final_consonant(text)
        let has_final: Bool = forms[0] == "으로" ? final != 0 && final != RIEUL : final != 0
        return text + (has_final ? forms[0] : forms[1])
    }

    private static func is_closing(_ scalar: Unicode.Scalar) -> Bool
    {
        return scalar == "\"" || scalar == "'" || scalar == "}" || scalar == "]" || scalar == ")"
            || scalar.properties.isWhitespace
    }
}
