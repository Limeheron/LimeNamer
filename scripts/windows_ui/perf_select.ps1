$script:APP_PATH = Join-Path $ROOT_DIR 'build\dotnet\bin\limenamer_app\release\LimeNamer.exe'
start_app
wait_for_display
$folder = make_many 'perf_100000' 100000
$button = open_folder_dialog $folder
[UI_NATIVE]::PostMessage($button, 0x00F5, [IntPtr]::Zero, [IntPtr]::Zero) | Out-Null
wait_until { (header_text) -match '^100000개' } 300 | Out-Null
add_insert_mod 'a_'
$text_type = New-Object System.Windows.Automation.PropertyCondition(
    $AE::ControlTypeProperty, [System.Windows.Automation.ControlType]::Text)
$all_texts = (main_window).FindAll($TREE::Descendants, $text_type)
$status = $null
for ($i = 0; $i -lt $all_texts.Count - 1; $i++) {
    if ($all_texts[$i].Current.Name -match '^\d\d:\d\d$') {
        $status = $all_texts[$i + 1]
    }
}
$grid = (find_all ([System.Windows.Automation.ControlType]::DataGrid))[0]
$toggle = @(find_all_named '켜기와 끄기' | Where-Object { -not $_.Current.IsOffscreen })[0]
$toggle_at = center_of $toggle

function cpu()
{
    $script:app.Refresh()
    return [Math]::Round($script:app.TotalProcessorTime.TotalSeconds, 2)
}

function step([string]$label, [string]$expect)
{
    $before = cpu
    mouse_click $toggle_at[0] $toggle_at[1]
    $wait = wait_until { $status.Current.Name -eq $expect } 300
    $used = [Math]::Round((cpu) - $before, 2)
    write_log "P06 $label message=$($wait[0])s longest=$($wait[1])s cpu+$($used)s"
}

step 'none-off' '수정을 껐습니다.'
step 'none-on' '수정을 켰습니다.'
$grid.SetFocus()
send_keys '^a'
Start-Sleep -Seconds 2
step 'all-off' '수정을 껐습니다.'
step 'all-on' '수정을 켰습니다.'
$grid.SetFocus()
send_keys '^{HOME}'
send_keys '{DOWN}'
send_keys '+^{END}'
Start-Sleep -Seconds 2
write_log "P06 part selected $(header_text)"
step 'part-off' '수정을 껐습니다.'
step 'part-on' '수정을 켰습니다.'
