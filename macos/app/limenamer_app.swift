import SwiftUI

@main
struct LIMENAMER_APP: App
{
    static let VERSION_TOKEN: String = "LIMENAMER_VERSION=0.1.0"

    var body: some Scene
    {
        Window("LimeNamer", id: "main") {
            Color.clear
                .frame(minWidth: 900, minHeight: 560)
        }
        .defaultSize(width: 1280, height: 800)
    }
}
