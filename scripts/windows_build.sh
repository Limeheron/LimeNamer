#!/bin/zsh
WINDOWS_BUILD_VERSION=0.1.0

readonly SCRIPT_DIR=${0:A:h}
readonly CONFIG_FILE=$SCRIPT_DIR/windows.conf
if [[ -r $CONFIG_FILE ]]; then
    source $CONFIG_FILE
fi
readonly HOST=${WINDOWS_HOST:-}
readonly ROOT_DIR=${WINDOWS_ROOT:-}
readonly VS_DIR=${WINDOWS_VS_DIR:-}
readonly DOTNET_DIR=${WINDOWS_DOTNET_DIR:-}
readonly VS_DEV_CMD="$VS_DIR\\Common7\\Tools\\VsDevCmd.bat"
readonly SOURCE_DIR="$ROOT_DIR\\source"
readonly ENGINE_DIR="$SOURCE_DIR\\engine"
readonly APP_DIR="$SOURCE_DIR\\windows"
readonly BUILD_ROOT="$ROOT_DIR\\build"
readonly TEMP_DIR="$ROOT_DIR\\temp"
readonly PACKAGE_DIR="$BUILD_ROOT\\package"
readonly DIST_DIR=${SCRIPT_DIR:h}/dist
readonly VERSION_FILE=${SCRIPT_DIR:h}/engine/src/session.h
readonly -a DOTNET_ENV=(
    "PATH=$DOTNET_DIR;%PATH%"
    DOTNET_CLI_TELEMETRY_OPTOUT=1
    TESTINGPLATFORM_TELEMETRY_OPTOUT=1
    DOTNET_NOLOGO=1
    DOTNET_CLI_UI_LANGUAGE=en
    DOTNET_ADD_GLOBAL_TOOLS_TO_PATH=false
    DOTNET_GENERATE_ASPNET_CERTIFICATE=false
    DOTNET_CLI_USE_MSBUILD_SERVER=false
    MSBUILDDISABLENODEREUSE=1
    "DOTNET_CLI_HOME=$BUILD_ROOT\\dotnet_home"
    "NUGET_PACKAGES=$BUILD_ROOT\\nuget\\packages"
    "NUGET_HTTP_CACHE_PATH=$BUILD_ROOT\\nuget\\http_cache"
    "NUGET_PLUGINS_CACHE_PATH=$BUILD_ROOT\\nuget\\plugins_cache"
)

usage()
{
    print -u2 "usage: windows_build.sh [-v | --version | debug | release | all | package]"
    exit 1
}

build()
{
    local config=$1
    local build_dir="$BUILD_ROOT\\win-$config"
    local dotnet_options="-c ${(C)config} --artifacts-path $BUILD_ROOT\\dotnet"
    local var
    local remote_command="chcp 65001>nul & set VSLANG=1033"
    remote_command+=" & call \"$VS_DEV_CMD\" -arch=x64 -host_arch=x64 -no_logo"
    remote_command+=" && cd /d $ENGINE_DIR"
    remote_command+=" && cmake --preset win-$config -B $build_dir"
    remote_command+=" && cmake --build $build_dir"
    remote_command+=" && ((if exist $TEMP_DIR rmdir /s /q $TEMP_DIR) & mkdir $TEMP_DIR)"
    remote_command+=" && set \"TEMP=$TEMP_DIR\" && set \"TMP=$TEMP_DIR\""
    remote_command+=" && ctest --test-dir $build_dir --output-on-failure"
    for var in $DOTNET_ENV; do
        remote_command+=" && set \"$var\""
    done
    remote_command+=" && (taskkill /im LimeNamer.exe /f >nul 2>&1 || ver >nul)"
    remote_command+=" && dotnet build $APP_DIR\\limenamer.slnx $dotnet_options"
    remote_command+=" -p:EngineBinDir=$build_dir\\bin -p:RestoreConfigFile=$APP_DIR\\nuget.config"
    remote_command+=" -p:UseSharedCompilation=false -nodeReuse:false"
    remote_command+=" && dotnet test $APP_DIR\\limenamer.slnx $dotnet_options --no-build"

    print -r -- "== win-$config"
    ssh -o BatchMode=yes $HOST $remote_command 2>&1 | LC_ALL=C tr -d '\r'
}

package()
{
    local version=$(sed -n 's/.*LIMENAMER_VERSION=\([0-9.]*\)".*/\1/p' $VERSION_FILE)
    local name="LimeNamer-$version-win-x64"
    local folder="$PACKAGE_DIR\\LimeNamer"
    local var
    local remote_command="chcp 65001>nul & ((if exist $PACKAGE_DIR rmdir /s /q $PACKAGE_DIR)"
    remote_command+=" & mkdir $PACKAGE_DIR)"
    if [[ -z $version ]]; then
        print -u2 "Version not found in $VERSION_FILE"
        exit 1
    fi
    for var in $DOTNET_ENV; do
        remote_command+=" && set \"$var\""
    done
    remote_command+=" && dotnet publish $APP_DIR\\app\\limenamer_app.csproj -c Release"
    remote_command+=" --artifacts-path $BUILD_ROOT\\dotnet -o $folder --self-contained false"
    remote_command+=" -p:EngineBinDir=$BUILD_ROOT\\win-release\\bin"
    remote_command+=" -p:RestoreConfigFile=$APP_DIR\\nuget.config -p:DebugType=none"
    remote_command+=" -p:UseSharedCompilation=false -nodeReuse:false"
    remote_command+=" && copy /y $SOURCE_DIR\\LICENSE $folder\\LICENSE >nul"
    remote_command+=" && xcopy /e /i /q /y $SOURCE_DIR\\licenses $folder\\licenses >nul"
    remote_command+=" && tar -a -c -f $PACKAGE_DIR\\$name.zip -C $PACKAGE_DIR LimeNamer"

    print -r -- "== package $name"
    ssh -o BatchMode=yes $HOST $remote_command 2>&1 | LC_ALL=C tr -d '\r'
    mkdir -p $DIST_DIR
    scp -q -o BatchMode=yes "$HOST:/${PACKAGE_DIR//\\//}/$name.zip" $DIST_DIR/
    chmod 644 $DIST_DIR/$name.zip
    print -r -- "Saved $DIST_DIR/$name.zip"
}

check_config()
{
    if [[ -z $HOST || -z $ROOT_DIR || -z $VS_DIR || -z $DOTNET_DIR ]]; then
        print -u2 "Set WINDOWS_HOST, WINDOWS_ROOT, WINDOWS_VS_DIR and WINDOWS_DOTNET_DIR" \
            "in $CONFIG_FILE (see windows.conf.example)"
        exit 1
    fi
}

main()
{
    local config

    check_config
    $SCRIPT_DIR/windows_sync.sh
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
                print $WINDOWS_BUILD_VERSION
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
