import Foundation

enum TEST_PATHS
{
    static let SOURCE_DIR: URL = URL(fileURLWithPath: #filePath).deletingLastPathComponent()
        .appendingPathComponent("../../..").standardizedFileURL
    static let TESTDATA_DIR: URL = SOURCE_DIR.appendingPathComponent("testdata")
    static let STRINGS_RESX: URL = SOURCE_DIR.appendingPathComponent(
        "windows/view_model/strings/strings.resx")
}
