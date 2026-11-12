import AppKit
import SwiftUI

enum PALETTE
{
    static let ACCENT_TEXT: Color = PALETTE.color(0x4B7A10, 0xA9D65F)
    static let ACCENT_FILL: Color = PALETTE.color(0x9CCF45, 0xA3D14B)
    static let ACCENT_ON_FILL: Color = PALETTE.color(0x17210A, 0x16200A)
    static let ACCENT_SOFT: Color = PALETTE.color(0xEEF6DF, 0x2B3520)
    static let OK_TEXT: Color = PALETTE.color(0x2F7A3A, 0x86CF8F)
    static let OK_BACK: Color = PALETTE.color(0xE2F1E3, 0x1B3220)
    static let WARNING_TEXT: Color = PALETTE.color(0x8F5600, 0xE5AB45)
    static let WARNING_BACK: Color = PALETTE.color(0xFBECD0, 0x3A2C10)
    static let ERROR_TEXT: Color = PALETTE.color(0xB42318, 0xF2766B)
    static let ERROR_BACK: Color = PALETTE.color(0xFBE1DD, 0x3E1A16)
    static let CONFLICT_TEXT: Color = PALETTE.color(0x6A3FC4, 0xB79EF6)
    static let CONFLICT_BACK: Color = PALETTE.color(0xECE5FB, 0x2B2242)
    static let FOLDER_TEXT: Color = PALETTE.color(0x22678E, 0x73B6DC)
    static let FOLDER_BACK: Color = PALETTE.color(0xDFEDF5, 0x15303F)

    private static func color(_ light: UInt32, _ dark: UInt32) -> Color
    {
        let dynamic: NSColor = NSColor(name: nil) { appearance in
            let match: NSAppearance.Name? = appearance.bestMatch(from: [.aqua, .darkAqua])
            return PALETTE.rgb(match == .darkAqua ? dark : light)
        }
        return Color(nsColor: dynamic)
    }

    private static func rgb(_ value: UInt32) -> NSColor
    {
        let red: CGFloat = CGFloat((value >> 16) & 0xFF) / 255
        let green: CGFloat = CGFloat((value >> 8) & 0xFF) / 255
        let blue: CGFloat = CGFloat(value & 0xFF) / 255
        return NSColor(srgbRed: red, green: green, blue: blue, alpha: 1)
    }
}
