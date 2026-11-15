import SwiftUI

struct NUMBER_INPUT: View
{
    private static let WIDTH: CGFloat = 72
    private static let ALLOWED: Set<Character> = ["-", "0", "1", "2", "3", "4", "5", "6", "7",
        "8", "9"]

    let value: Int
    let range: ClosedRange<Int>
    var fills: Bool = false
    let changed: (Int) -> Void

    @State private var text: String = ""
    @FocusState private var focused: Bool

    var body: some View
    {
        HStack(spacing: 4) {
            FIELD_FRAME(focused: focused, problem: "") {
                TextField("", text: $text)
                    .textFieldStyle(.plain)
                    .font(TEXT_INPUT.FONT)
                    .autocorrectionDisabled()
                    .padding(.horizontal, 10)
                    .focused($focused)
            }
            .frame(width: fills ? nil : NUMBER_INPUT.WIDTH)
            .frame(maxWidth: fills ? .infinity : nil)
            Stepper("", value: stepper_value, in: range)
                .labelsHidden()
        }
        .onAppear {
            text = String(value)
        }
        .onChange(of: value) { _, current in
            if !focused && Int(text) != current {
                text = String(current)
            }
        }
        .onChange(of: text) { _, current in
            typed(current)
        }
        .onChange(of: focused) { _, now in
            if !now {
                text = String(value)
            }
        }
    }

    private var stepper_value: Binding<Int>
    {
        return Binding(get: {
            return value
        }, set: { current in
            text = String(current)
            changed(current)
        })
    }

    private func typed(_ current: String)
    {
        let filtered: String = current.filter { NUMBER_INPUT.ALLOWED.contains($0) }
        if filtered != current {
            text = filtered
            return
        }
        guard let parsed = Int(filtered) else {
            return
        }
        let clamped: Int = min(max(parsed, range.lowerBound), range.upperBound)
        if clamped != value {
            changed(clamped)
        }
    }
}
