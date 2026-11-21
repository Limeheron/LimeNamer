#!/bin/zsh
WINDOWS_SYNC_VERSION=0.1.0

readonly SCRIPT_DIR=${0:A:h}
readonly SOURCE_DIR=${SCRIPT_DIR:h}
readonly CONFIG_FILE=$SCRIPT_DIR/windows.conf
if [[ -r $CONFIG_FILE ]]; then
    source $CONFIG_FILE
fi
readonly HOST=${WINDOWS_HOST:-}
readonly TARGET_DIR=${WINDOWS_ROOT:+$WINDOWS_ROOT\\source}

usage()
{
    print -u2 "usage: windows_sync.sh [-v | --version]"
    exit 1
}

check_config()
{
    if [[ -z $HOST || -z $TARGET_DIR ]]; then
        print -u2 "Set WINDOWS_HOST and WINDOWS_ROOT in $CONFIG_FILE (see windows.conf.example)"
        exit 1
    fi
}

main()
{
    local -a files
    local file
    local remote_command="(if exist $TARGET_DIR rmdir /s /q $TARGET_DIR)"
    remote_command+=" & mkdir $TARGET_DIR && tar -xf - -C $TARGET_DIR"

    check_config
    cd $SOURCE_DIR
    for file in ${(0)"$(git ls-files -co --exclude-standard -z)"}; do
        if [[ -e $file ]]; then
            files+=($file)
        fi
    done
    if (( ${#files} == 0 )); then
        print -u2 "No files to sync"
        exit 1
    fi

    print -rN -- $files | COPYFILE_DISABLE=1 tar --no-xattrs --null -T - -cf - \
        | ssh -o BatchMode=yes $HOST $remote_command
    print -r -- "Synced ${#files} files to $HOST:$TARGET_DIR"
}

setopt err_exit pipe_fail

case $# in
    0)
        main
        ;;
    1)
        if [[ $1 == -v || $1 == --version ]]; then
            print $WINDOWS_SYNC_VERSION
        else
            usage
        fi
        ;;
    *)
        usage
        ;;
esac
