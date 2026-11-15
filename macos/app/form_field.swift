import SwiftUI

struct FORM_FIELD<Content: View>: View
{
    let label: String
    @ViewBuilder let content: Content

    var body: some View
    {
        VStack(alignment: .leading, spacing: 6) {
            Text(label)
                .font(.system(size: 12, weight: .semibold))
                .foregroundStyle(.secondary)
            content
        }
        .frame(maxWidth: .infinity, alignment: .leading)
    }
}
