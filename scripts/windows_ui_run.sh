#!/bin/zsh
WINDOWS_UI_RUN_VERSION=0.1.0

readonly SCRIPT_DIR=${0:A:h}
readonly CONFIG_FILE=$SCRIPT_DIR/windows.conf
if [[ -r $CONFIG_FILE ]]; then
    source $CONFIG_FILE
fi
readonly HOST=${WINDOWS_HOST:-}
readonly WORK_DIR="${WINDOWS_ROOT:-}\\temp\\ui_test"
readonly TASK_NAME=limenamer_ui_test

usage()
{
    print -u2 "usage: windows_ui_run.sh [-v | --version] <case> <output folder>"
    exit 1
}

run_case()
{
    local case=$1
    local output=$2
    local remote_dir="/${WORK_DIR//\\//}"
    local tries

    local start="(if not exist $WORK_DIR mkdir $WORK_DIR)"
    start+=" & del /q $WORK_DIR\\done.txt $WORK_DIR\\*.png 2>nul"
    start+=" & echo $case> $WORK_DIR\\case.txt & schtasks /run /tn $TASK_NAME >nul"

    $SCRIPT_DIR/windows_sync.sh
    ssh -o BatchMode=yes $HOST $start
    for tries in {1..600}; do
        if ssh -o BatchMode=yes $HOST "if exist $WORK_DIR\\done.txt (exit 0) else (exit 1)"; then
            break
        fi
        sleep 1
    done
    mkdir -p $output
    scp -q -o BatchMode=yes "$HOST:$remote_dir/log.txt" "$HOST:$remote_dir/*.png" $output/ \
        2>/dev/null
    cat $output/log.txt
}

case $# in
    1)
        [[ $1 == -v || $1 == --version ]] && print $WINDOWS_UI_RUN_VERSION || usage
        ;;
    2)
        run_case $1 $2
        ;;
    *)
        usage
        ;;
esac
