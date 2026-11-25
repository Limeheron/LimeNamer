$script:APP_PATH = Join-Path $ROOT_DIR 'build\dotnet\bin\limenamer_app\release\LimeNamer.exe'
$large = make_many 'perf_100k' 100000
start_app
wait_for_display
$button = open_folder_dialog $large
[UI_NATIVE]::PostMessage($button, 0x00F5, [IntPtr]::Zero, [IntPtr]::Zero) | Out-Null
$add = wait_until { (header_text) -match '^100000개' } 300
write_log "R01 add $($add[0])s"
add_insert_mod 'a_'
$preview = wait_until { @(new_names)[0] -like 'a_*' } 120
write_log "R01 apply-mod $($preview[0])s"

$handle = $script:app.MainWindowHandle
$ping = { [void][UI_NATIVE]::SendMessage($handle, 0, [IntPtr]::Zero, [IntPtr]::Zero); $false }
$idle = wait_until $ping 5
write_log "R02 idle longest-unresponsive $($idle[1])s"
$info = find_element '정보' (main_window)
for ($round = 1; $round -le 3; $round++) {
    front
    $info.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
    $dialog = wait_dialog
    $ok = find_element '확인' $dialog
    front ([IntPtr]$dialog.Current.NativeWindowHandle)
    $ok.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
    $reread = wait_until $ping 10
    write_log "R02 round $round longest-unresponsive $($reread[1])s"
}
