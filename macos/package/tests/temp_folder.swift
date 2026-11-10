import Foundation

final class TEMP_FOLDER
{
    let path: String

    init() throws
    {
        path = NSTemporaryDirectory() + "limenamer_tests_" + UUID().uuidString
        try FileManager.default.createDirectory(atPath: path, withIntermediateDirectories: false)
    }

    func file(_ name: String, _ content: String = "") throws -> String
    {
        let full_path: String = path + "/" + name
        try Data(content.utf8).write(to: URL(fileURLWithPath: full_path))
        return full_path
    }

    func folder(_ name: String) throws -> String
    {
        let full_path: String = path + "/" + name
        try FileManager.default.createDirectory(atPath: full_path,
            withIntermediateDirectories: false)
        return full_path
    }

    func exists(_ name: String) -> Bool
    {
        return FileManager.default.fileExists(atPath: path + "/" + name)
    }

    func remove()
    {
        try? FileManager.default.removeItem(atPath: path)
    }
}
