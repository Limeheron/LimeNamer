import SwiftUI
import UniformTypeIdentifiers

import limenamer_view_model

struct FILE_DROP: DropDelegate
{
    let main: MAIN_VIEW_MODEL
    let targeted: Binding<Bool>

    func validateDrop(info: DropInfo) -> Bool
    {
        return main.can_edit && info.hasItemsConforming(to: [.fileURL])
    }

    func dropEntered(info: DropInfo)
    {
        targeted.wrappedValue = validateDrop(info: info)
    }

    func dropUpdated(info: DropInfo) -> DropProposal?
    {
        return DropProposal(operation: validateDrop(info: info) ? .copy : .forbidden)
    }

    func dropExited(info: DropInfo)
    {
        targeted.wrappedValue = false
    }

    func performDrop(info: DropInfo) -> Bool
    {
        targeted.wrappedValue = false
        if !validateDrop(info: info) {
            return false
        }
        let providers: [NSItemProvider] = info.itemProviders(for: [.fileURL])
        let main: MAIN_VIEW_MODEL = main
        Task { @MainActor in
            var paths: [String] = []
            for provider in providers {
                if let path = await FILE_DROP.path(provider) {
                    paths.append(path)
                }
            }
            main.add_paths(paths, mode: .as_items)
        }
        return true
    }

    private static func path(_ provider: NSItemProvider) async -> String?
    {
        let data: Data? = await withCheckedContinuation { continuation in
            _ = provider.loadDataRepresentation(for: .fileURL) { data, _ in
                continuation.resume(returning: data)
            }
        }
        guard let data, let url = URL(dataRepresentation: data, relativeTo: nil) else {
            return nil
        }
        return url.path(percentEncoded: false)
    }
}
