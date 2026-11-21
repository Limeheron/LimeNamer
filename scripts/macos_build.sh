#!/bin/zsh
MACOS_BUILD_VERSION=0.1.0

readonly SCRIPT_DIR=${0:A:h}
readonly ENGINE_DIR=${SCRIPT_DIR:h}/engine
readonly MACOS_DIR=${SCRIPT_DIR:h}/macos
readonly BUILD_DIR=$MACOS_DIR/build
readonly DIST_DIR=${SCRIPT_DIR:h}/dist
readonly VERSION_FILE=$ENGINE_DIR/src/session.h
readonly -a SWIFTPM_OPTIONS=(
    --package-path $MACOS_DIR/package
    --cache-path $BUILD_DIR/swiftpm/cache
    --config-path $BUILD_DIR/swiftpm/config
    --security-path $BUILD_DIR/swiftpm/security
)

usage()
{
    print -u2 "usage: macos_build.sh [-v | --version | debug | release | all | package]"
    exit 1
}

build()
{
    local config=$1
    local -a test_options=($SWIFTPM_OPTIONS --scratch-path $BUILD_DIR/swiftpm/$config)

    print -r -- "== mac-$config"
    cmake --preset mac-$config
    cmake --build --preset mac-$config --parallel
    ctest --preset mac-$config
    if [[ $config == debug ]]; then
        cmake --preset mac-release
        cmake --build --preset mac-release --parallel --target limenamer_engine
        test_options+=(--sanitize address)
    else
        test_options+=(-c release)
    fi
    swift test $test_options
    xcodebuild -project $MACOS_DIR/limenamer.xcodeproj -scheme limenamer_app \
        -configuration ${(C)config} -destination 'platform=macOS,arch=arm64' \
        -derivedDataPath $BUILD_DIR/xcode -quiet build
}

package()
{
    local version=$(sed -n 's/.*LIMENAMER_VERSION=\([0-9.]*\)".*/\1/p' $VERSION_FILE)
    local name="LimeNamer-$version-mac-arm64"
    local app=$BUILD_DIR/xcode/Build/Products/Release/LimeNamer.app
    local stage=$BUILD_DIR/package

    if [[ -z $version ]]; then
        print -u2 "Version not found in $VERSION_FILE"
        exit 1
    fi
    print -r -- "== package $name"
    rm -rf $stage
    mkdir -p $stage $DIST_DIR
    ditto $app $stage/LimeNamer.app
    ln -s /Applications $stage/Applications
    rm -f $DIST_DIR/$name.dmg
    hdiutil create -quiet -volname LimeNamer -srcfolder $stage -format UDZO $DIST_DIR/$name.dmg
    rm -rf $stage
    print -r -- "Saved $DIST_DIR/$name.dmg"
}

main()
{
    local config

    swift -module-cache-path $BUILD_DIR/module_cache $MACOS_DIR/tools/generate_strings.swift
    cd $ENGINE_DIR
    for config in $@; do
        build $config
    done
}

setopt err_exit pipe_fail

case $# in
    0)
        main debug
        ;;
    1)
        case $1 in
            -v|--version)
                print $MACOS_BUILD_VERSION
                ;;
            debug|release)
                main $1
                ;;
            all)
                main debug release
                ;;
            package)
                main release
                package
                ;;
            *)
                usage
                ;;
        esac
        ;;
    *)
        usage
        ;;
esac
