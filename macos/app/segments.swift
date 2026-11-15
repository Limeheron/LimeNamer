import AppKit
import SwiftUI

struct SEGMENTS: NSViewRepresentable
{
    let labels: [String]
    let tips: [String]
    let selected: Int
    let changed: (Int) -> Void

    func makeCoordinator() -> ACTION_TARGET
    {
        return ACTION_TARGET()
    }

    func makeNSView(context: Context) -> NSSegmentedControl
    {
        let control: NSSegmentedControl = NSSegmentedControl(labels: labels,
            trackingMode: .selectOne, target: context.coordinator,
            action: #selector(ACTION_TARGET.fire(_:)))
        control.segmentDistribution = .fillEqually
        for (index, tip) in tips.enumerated() {
            control.setToolTip(tip, forSegment: index)
        }
        updateNSView(control, context: context)
        return control
    }

    func updateNSView(_ control: NSSegmentedControl, context: Context)
    {
        let changed: (Int) -> Void = changed
        context.coordinator.action = { [weak control] in
            if let control {
                changed(control.selectedSegment)
            }
        }
        if control.selectedSegment != selected {
            control.selectedSegment = selected
        }
        control.isEnabled = context.environment.isEnabled
    }

    func sizeThatFits(_ proposal: ProposedViewSize, nsView control: NSSegmentedControl,
        context: Context) -> CGSize?
    {
        let natural: NSSize = control.intrinsicContentSize
        return CGSize(width: proposal.width ?? natural.width, height: natural.height)
    }
}
