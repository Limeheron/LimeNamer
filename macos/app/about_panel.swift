import AppKit

import limenamer_view_model

@MainActor
enum ABOUT_PANEL
{
    private static let JSON_NAME: String = "nlohmann/json 3.12.0"
    private static let JSON_URL: String = "https://github.com/nlohmann/json"
    private static let FONT: NSFont = NSFont.systemFont(ofSize: 11)
    private static let BOLD_FONT: NSFont = NSFont.boldSystemFont(ofSize: 11)

    static func show(version: String)
    {
        NSApp.orderFrontStandardAboutPanel(options: [
            .applicationVersion: version,
            .version: "",
            .credits: credits(),
        ])
        NSApp.activate()
    }

    private static func credits() -> NSAttributedString
    {
        let text: NSMutableAttributedString = NSMutableAttributedString()
        text.append(line(STRINGS.info_description, FONT))
        text.append(line(STRINGS.info_license, FONT))
        text.append(line("", FONT))
        text.append(line(STRINGS.info_notices, BOLD_FONT))
        text.append(line("", FONT))
        text.append(line("LimeNamer", BOLD_FONT))
        text.append(line(read("LICENSE"), FONT))
        text.append(line(JSON_NAME, BOLD_FONT))
        text.append(line(JSON_URL, FONT, link: URL(string: JSON_URL)))
        text.append(line("", FONT))
        text.append(line(read("nlohmann_json.txt"), FONT))
        return text
    }

    private static func line(_ value: String, _ font: NSFont, link: URL? = nil)
        -> NSAttributedString
    {
        var attributes: [NSAttributedString.Key: Any] = [.font: font,
            .foregroundColor: NSColor.labelColor]
        if let link {
            attributes[.link] = link
        }
        return NSAttributedString(string: value.trimmingCharacters(in: .newlines) + "\n",
            attributes: attributes)
    }

    private static func read(_ name: String) -> String
    {
        guard let url = Bundle.main.url(forResource: name, withExtension: nil),
            let text = try? String(contentsOf: url, encoding: .utf8) else {
            return ""
        }
        return text
    }
}
