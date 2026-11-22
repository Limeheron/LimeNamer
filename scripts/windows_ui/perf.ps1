$script:APP_PATH = Join-Path $ROOT_DIR 'build\dotnet\bin\limenamer_app\release\LimeNamer.exe'
$small = make_many 'perf_10k' 10000
$large = make_many 'perf_100k' 100000
start_app
wait_for_display
write_log "P01 start memory $(memory_mb)MB"

foreach ($run in @(@($small, 10000), @($large, 100000))) {
    $folder = $run[0]
    $count = $run[1]
    $button = open_folder_dialog $folder
    [UI_NATIVE]::PostMessage($button, 0x00F5, [IntPtr]::Zero, [IntPtr]::Zero) | Out-Null
    $add = wait_until { (header_text) -match "^$count개" } 300
    write_log "P02 $count add $($add[0])s longest-unresponsive $($add[1])s memory $(memory_mb)MB"
    add_insert_mod 'a_'
    $preview = wait_until { @(new_names)[0] -like 'a_*' } 120
    write_log "P03 $count apply-mod $($preview[0])s longest $($preview[1])s"
    press '이름 바꾸기'
    $rename = wait_until { (status_message) -match '이름을 바꿨습니다' } 600
    write_log "P04 $count rename $($rename[0])s longest $($rename[1])s"
    press '이름 되돌리기'
    $dialog = wait_dialog
    press_in $dialog '이름 되돌리기'
    $undo = wait_until { (status_message) -match '되돌렸습니다' } 600
    write_log "P04 $count undo $($undo[0])s longest $($undo[1])s memory $(memory_mb)MB"
    if ($count -eq 100000) {
        (find_all ([System.Windows.Automation.ControlType]::DataGrid))[0].SetFocus()
        $end = wait_until { send_keys '^{END}'; $true } 30
        capture 'p05'
        write_log "P05 jump-to-end $($end[0])s"
        $off = wait_until { click_named '켜기와 끄기'; $true } 60
        $none = wait_until { @(new_names)[0] -like 'a_*' } 60
        send_keys '^a'
        click_named '켜기와 끄기'
        $all = wait_until { -not (@(new_names)[0] -like 'a_*') } 60
        write_log "P06 toggle with none selected $($none[0])s, with all selected $($all[0])s"
    }
    press '비우기'
    Start-Sleep -Seconds 2
}

$base = memory_mb
for ($round = 1; $round -le 5; $round++) {
    $button = open_folder_dialog $small
    [UI_NATIVE]::PostMessage($button, 0x00F5, [IntPtr]::Zero, [IntPtr]::Zero) | Out-Null
    wait_until { (header_text) -match '^10000개' } 120 | Out-Null
    $added = memory_mb
    press '비우기'
    Start-Sleep -Seconds 2
    write_log "P07 round $round after add $($added)MB after clear $(memory_mb)MB"
}
