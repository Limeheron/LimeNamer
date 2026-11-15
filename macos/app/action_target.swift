import AppKit

@MainActor
final class ACTION_TARGET: NSObject
{
    var action: () -> Void = {}

    @objc func fire(_ sender: Any?)
    {
        action()
    }
}
