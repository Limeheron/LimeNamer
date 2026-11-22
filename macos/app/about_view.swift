import AppKit
import SwiftUI

import limenamer_view_model

struct ABOUT_VIEW: View
{
    private static let WIDTH: CGFloat = 300
    private static let ICON: CGFloat = 64
    private static let PROFILE: CGFloat = 44
    private static let NOTICES_HEIGHT: CGFloat = 160
    private static let RADIUS: CGFloat = 6
    private static let FONT: Font = .system(size: 11)
    private static let BOLD_FONT: Font = .system(size: 11, weight: .bold)
    private static let JSON_NAME: String = "nlohmann/json 3.12.0"
    private static let JSON_URL: String = "https://github.com/nlohmann/json"
    private static let PROFILE_NAME: String = "Limeheron"
    private static let PROFILE_URL: String = "https://github.com/Limeheron"
    private static let PROFILE_LINK: String = "github.com/Limeheron"
    private static let LIMENAMER_LICENSE: String = ABOUT_VIEW.license("LICENSE")
    private static let JSON_LICENSE: String = ABOUT_VIEW.license("nlohmann_json.txt")

    let version: String

    var body: some View
    {
        VStack(spacing: 0) {
            Image(nsImage: NSApp.applicationIconImage)
                .resizable()
                .frame(width: ABOUT_VIEW.ICON, height: ABOUT_VIEW.ICON)
            Text("LimeNamer")
                .font(.system(size: 14, weight: .bold))
                .padding(.top, 8)
            Text(JOSA_FORMATTER.format(STRINGS.info_version, version))
                .foregroundStyle(.secondary)
                .padding(.top, 2)
            Text(STRINGS.info_notices)
                .font(ABOUT_VIEW.BOLD_FONT)
                .frame(maxWidth: .infinity, alignment: .leading)
                .padding(EdgeInsets(top: 16, leading: 0, bottom: 6, trailing: 0))
            notices
            profile
                .padding(.top, 14)
        }
        .font(ABOUT_VIEW.FONT)
        .padding(EdgeInsets(top: 4, leading: 20, bottom: 18, trailing: 20))
        .frame(width: ABOUT_VIEW.WIDTH)
    }

    private var notices: some View
    {
        let shape: RoundedRectangle = RoundedRectangle(cornerRadius: ABOUT_VIEW.RADIUS)
        return ScrollView {
            VStack(alignment: .leading, spacing: 0) {
                Text("LimeNamer")
                    .font(ABOUT_VIEW.BOLD_FONT)
                Text(ABOUT_VIEW.LIMENAMER_LICENSE)
                Text(ABOUT_VIEW.JSON_NAME)
                    .font(ABOUT_VIEW.BOLD_FONT)
                    .padding(.top, 13)
                link(ABOUT_VIEW.JSON_URL, ABOUT_VIEW.JSON_URL)
                Text(ABOUT_VIEW.JSON_LICENSE)
            }
            .textSelection(.enabled)
            .frame(maxWidth: .infinity, alignment: .leading)
            .padding(EdgeInsets(top: 8, leading: 10, bottom: 8, trailing: 10))
        }
        .frame(height: ABOUT_VIEW.NOTICES_HEIGHT)
        .background(shape.fill(Color(nsColor: PALETTE.FIELD_BACK)))
        .overlay(shape.strokeBorder(Color(nsColor: PALETTE.FIELD_LINE), lineWidth: 1))
    }

    private var profile: some View
    {
        HStack(spacing: 10) {
            Image(nsImage: NSImage(named: "profile") ?? NSImage())
                .resizable()
                .frame(width: ABOUT_VIEW.PROFILE, height: ABOUT_VIEW.PROFILE)
            VStack(alignment: .leading, spacing: 1) {
                Text(ABOUT_VIEW.PROFILE_NAME)
                    .font(ABOUT_VIEW.BOLD_FONT)
                link(ABOUT_VIEW.PROFILE_LINK, ABOUT_VIEW.PROFILE_URL)
            }
        }
    }

    private func link(_ title: String, _ url: String) -> some View
    {
        return Link(title, destination: URL(string: url)!)
    }

    private static func license(_ name: String) -> String
    {
        guard let url = Bundle.main.url(forResource: name, withExtension: nil),
            let text = try? String(contentsOf: url, encoding: .utf8) else {
            return ""
        }
        return text.trimmingCharacters(in: .whitespacesAndNewlines)
            .components(separatedBy: "\n\n")
            .map { $0.replacingOccurrences(of: "\n", with: " ") }
            .joined(separator: "\n\n")
    }
}
