// swift-tools-version: 6.2

import PackageDescription

let ENGINE_LIB_DIR = Context.packageDirectory + "/../../engine/build/mac-release/lib"

let package = Package(
    name: "limenamer",
    platforms: [.macOS(.v14)],
    products: [
        .library(name: "limenamer_view_model", targets: ["limenamer_view_model"]),
    ],
    targets: [
        .systemLibrary(name: "limenamer_engine", path: "engine_module"),
        .target(
            name: "limenamer_view_model",
            dependencies: ["limenamer_engine"],
            path: "view_model",
            swiftSettings: [.treatAllWarnings(as: .error)],
            linkerSettings: [
                .unsafeFlags(["-L", ENGINE_LIB_DIR]),
                .linkedLibrary("limenamer_engine"),
                .linkedLibrary("c++"),
                .linkedFramework("CoreFoundation"),
            ]),
        .testTarget(
            name: "limenamer_tests",
            dependencies: ["limenamer_engine", "limenamer_view_model"],
            path: "tests",
            swiftSettings: [.treatAllWarnings(as: .error)]),
    ]
)
