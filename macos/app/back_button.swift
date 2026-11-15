import SwiftUI

struct BACK_BUTTON: View
{
    let label: String
    let action: () -> Void

    var body: some View
    {
        Button(action: action) {
            Label(label, systemImage: "chevron.left")
                .foregroundStyle(.primary)
        }
        .buttonStyle(.accessoryBar)
    }
}
