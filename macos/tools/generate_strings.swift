#!/usr/bin/env swift
import Foundation

let GENERATE_STRINGS_VERSION: String = "0.1.0"

let SOURCE_LANGUAGE: String = "ko"
let MACOS_DIR: URL = URL(fileURLWithPath: #filePath).deletingLastPathComponent()
    .deletingLastPathComponent()
let RESX_DIR: URL = MACOS_DIR.appendingPathComponent("../windows/view_model/strings")
let OUTPUT_DIR: URL = MACOS_DIR.appendingPathComponent("package/view_model/strings")
let SWIFT_KEYWORDS: Set<String> = [
    "as", "associatedtype", "break", "case", "catch", "class", "continue", "default", "defer",
    "deinit", "do", "else", "enum", "extension", "fallthrough", "false", "fileprivate", "for",
    "func", "guard", "if", "import", "in", "init", "inout", "internal", "is", "let", "nil",
    "open", "operator", "private", "protocol", "public", "repeat", "rethrows", "return", "self",
    "static", "struct", "subscript", "super", "switch", "text", "throw", "throws", "true", "try",
    "typealias", "var", "where", "while",
]

final class RESX_READER: NSObject, XMLParserDelegate
{
    private(set) var entries: [(key: String, value: String)] = []
    private var key: String? = nil
    private var value: String? = nil

    func parser(_ parser: XMLParser, didStartElement element: String, namespaceURI: String?,
        qualifiedName: String?, attributes: [String: String] = [:])
    {
        if element == "data" {
            key = attributes["name"]
        } else if element == "value" && key != nil {
            value = ""
        }
    }

    func parser(_ parser: XMLParser, foundCharacters string: String)
    {
        if value != nil {
            value! += string
        }
    }

    func parser(_ parser: XMLParser, didEndElement element: String, namespaceURI: String?,
        qualifiedName: String?)
    {
        if element == "value", let key, let value {
            entries.append((key, value))
            self.value = nil
        } else if element == "data" {
            key = nil
        }
    }
}

func usage() -> Never
{
    FileHandle.standardError.write(Data("usage: generate_strings.swift [-v | --version]\n".utf8))
    exit(1)
}

func fail(_ message: String) -> Never
{
    FileHandle.standardError.write(Data("generate_strings: \(message)\n".utf8))
    exit(1)
}

func read_resx(_ url: URL) -> [(key: String, value: String)]
{
    guard let parser = XMLParser(contentsOf: url) else {
        fail("cannot read \(url.path)")
    }
    let reader = RESX_READER()
    parser.delegate = reader
    guard parser.parse() else {
        fail("invalid XML in \(url.path)")
    }
    return reader.entries
}

func languages() -> [(language: String, url: URL)]
{
    var result: [(language: String, url: URL)] = [
        (SOURCE_LANGUAGE, RESX_DIR.appendingPathComponent("strings.resx")),
    ]
    let names: [String] = (try? FileManager.default.contentsOfDirectory(atPath: RESX_DIR.path))
        ?? []
    for name in names.sorted() where name.hasPrefix("strings.") && name.hasSuffix(".resx") {
        let language: String = String(name.dropFirst("strings.".count).dropLast(".resx".count))
        if !language.isEmpty {
            result.append((language, RESX_DIR.appendingPathComponent(name)))
        }
    }
    return result
}

func write_catalog(_ keys: [String], _ tables: [String: [String: String]])
{
    var strings: [String: Any] = [:]
    for key in keys {
        var localizations: [String: Any] = [:]
        for (language, table) in tables {
            if let value = table[key] {
                localizations[language] = ["stringUnit": ["state": "translated", "value": value]]
            }
        }
        strings[key] = ["extractionState": "manual", "localizations": localizations]
    }
    let catalog: [String: Any] = [
        "sourceLanguage": SOURCE_LANGUAGE,
        "strings": strings,
        "version": "1.0",
    ]
    guard let data = try? JSONSerialization.data(withJSONObject: catalog,
        options: [.prettyPrinted, .sortedKeys, .withoutEscapingSlashes]) else {
        fail("cannot encode the string catalog")
    }
    write(data + Data("\n".utf8), "strings.xcstrings")
}

func write_accessors(_ keys: [String])
{
    var lines: [String] = ["import Foundation", "", "public enum STRINGS", "{"]
    for key in keys {
        let line: String = "    public static let \(key): String = text(\"\(key)\")"
        if line.count <= 100 {
            lines.append(line)
        } else {
            lines += ["    public static let \(key): String =", "        text(\"\(key)\")"]
        }
    }
    lines += [
        "",
        "    public static func text(_ key: String) -> String",
        "    {",
        "        return Bundle.module.localizedString(forKey: key, value: nil, table: \"strings\")",
        "    }",
        "}",
        "",
    ]
    write(Data(lines.joined(separator: "\n").utf8), "strings.swift")
}

func write(_ data: Data, _ name: String)
{
    let url: URL = OUTPUT_DIR.appendingPathComponent(name)
    if (try? Data(contentsOf: url)) == data {
        return
    }
    do {
        try FileManager.default.createDirectory(at: OUTPUT_DIR, withIntermediateDirectories: true)
        try data.write(to: url)
    } catch {
        fail("cannot write \(url.path)")
    }
}

func main()
{
    var keys: [String] = []
    var tables: [String: [String: String]] = [:]
    for (language, url) in languages() {
        var table: [String: String] = [:]
        for entry in read_resx(url) {
            table[entry.key] = entry.value
            if language == SOURCE_LANGUAGE {
                keys.append(entry.key)
            }
        }
        tables[language] = table
    }
    for key in keys {
        if key.wholeMatch(of: #/[a-z_][a-z0-9_]*/#) == nil || SWIFT_KEYWORDS.contains(key) {
            fail("unusable key \(key)")
        }
    }
    write_catalog(keys, tables)
    write_accessors(keys)
}

switch CommandLine.arguments.count {
case 1:
    main()
case 2 where ["-v", "--version"].contains(CommandLine.arguments[1]):
    print(GENERATE_STRINGS_VERSION)
default:
    usage()
}
