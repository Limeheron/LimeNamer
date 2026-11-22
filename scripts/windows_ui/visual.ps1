backup_settings
start_app
wait_for_display
capture 'g34_dark'
menu '화면 모드 메뉴' '밝게'
Start-Sleep -Milliseconds 800
capture 'g34_light'
press '정보'
$dialog = wait_dialog
capture 'g35' ([IntPtr]$dialog.Current.NativeWindowHandle)
press_in $dialog '확인'
menu '화면 모드 메뉴' '어둡게'

$fixture = make_fixture 'gui_fixture'
pick_path '폴더 안의 파일 추가...' $fixture
$resize = (main_window).GetCurrentPattern([System.Windows.Automation.TransformPattern]::Pattern)
$resize.Resize(900, 700)
Start-Sleep -Milliseconds 800
$narrow = (texts) -contains '정렬'
capture 'w01_narrow'
$resize.Resize(1280, 800)
Start-Sleep -Milliseconds 800
$wide = (texts) -contains '정렬'
write_log "W01 toolbar labels narrow=$narrow wide=$wide (see w01_narrow)"

(find_all ([System.Windows.Automation.ControlType]::DataGrid))[0].SetFocus()
send_keys '^a'
$all = header_text
send_keys '{F2}'
$editing = $AE::FocusedElement.Current.ControlType -eq [System.Windows.Automation.ControlType]::Edit
send_keys '{ESC}'
check 'W02' (($all -match '선택 8개') -and $editing) "$all editing=$editing"

direct_edit 0 '.hidden_a'
press '이름 바꾸기'
$dialog = wait_dialog
capture 'w03' ([IntPtr]$dialog.Current.NativeWindowHandle)
press_in $dialog '취소'
write_log 'W03 see w03'
