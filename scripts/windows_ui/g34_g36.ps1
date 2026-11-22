backup_settings
start_app
menu '화면 모드 메뉴' '밝게'
$saved = (Get-Content $SETTINGS_FILE -Raw | ConvertFrom-Json).theme_mode
close_app
start_app
$after = (Get-Content $SETTINGS_FILE -Raw | ConvertFrom-Json).theme_mode
check 'G34' (($saved -eq 'light') -and ($after -eq 'light')) "$saved / $after"

press '정보'
$dialog = wait_dialog
$about = dialog_texts $dialog
$passed = ($about -contains 'LimeNamer') -and ($about -contains '버전 1.0.0') -and
    ($about -contains '오픈소스 라이선스') -and ($about -contains 'Limeheron')
check 'G35' $passed ($about -join ' / ')
press_in $dialog '확인'

$many = make_many 'close_test' 20000
pick_path '폴더 안의 파일 추가...' $many
add_insert_mod 'a_'
press '이름 바꾸기'
Start-Sleep -Milliseconds 800
front
[System.Windows.Forms.SendKeys]::SendWait('%{F4}')
Start-Sleep -Milliseconds 500
$blocked = status_message
$alive = -not $script:app.HasExited
wait_until { (status_message) -match '이름을 바꿨습니다' } 60 | Out-Null
$finished = status_message
$script:app.CloseMainWindow() | Out-Null
$closed = $script:app.WaitForExit(10000)
$passed = $alive -and ($blocked -eq '이름을 바꾸거나 되돌리는 중에는 닫을 수 없습니다.') -and $closed
check 'G36' $passed "$blocked | $finished | closed=$closed"
Remove-Item -Recurse -Force $many
