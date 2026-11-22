start_app
$fixture = make_fixture 'gui_fixture'
pick_path '폴더 안의 파일 추가...' $fixture
add_insert_mod 'a_'
press '이름 바꾸기'
Start-Sleep -Milliseconds 800
$message = status_message
$passed = (Test-Path (Join-Path $fixture 'a_report.txt')) -and
    ($message -match '^8개 항목의 이름을 바꿨습니다\. 다시 적용되지 않게 수정을 모두 껐습니다')
check 'G27' $passed $message

direct_edit 0 '.hidden_a'
press '이름 바꾸기'
$dialog = wait_dialog
$lines = dialog_texts $dialog
capture 'g28' ([IntPtr]$dialog.Current.NativeWindowHandle)
press_in $dialog '취소'
$kept = Test-Path (Join-Path $fixture 'a_archive.tar.gz')
press '이름 바꾸기'
$dialog = wait_dialog
press_in $dialog '이름 바꾸기'
$passed = ($lines -contains '경고가 있는 항목이 있습니다') -and $kept -and
    (Test-Path (Join-Path $fixture '.hidden_a'))
check 'G28' $passed ($lines -join ' / ')

press '이름 되돌리기'
$dialog = wait_dialog
$confirm = dialog_texts $dialog
press_in $dialog '취소'
$cancelled = (Test-Path (Join-Path $fixture '.hidden_a')) -and (is_enabled '이름 되돌리기')
check 'G30' $cancelled ($confirm -join ' / ')
press '이름 되돌리기'
$dialog = wait_dialog
press_in $dialog '이름 되돌리기'
$message = status_message
$passed = (Test-Path (Join-Path $fixture 'a_archive.tar.gz')) -and
    ($message -eq '1개 항목을 실행 전 이름으로 되돌렸습니다.')
check 'G29' $passed $message

click_named '켜기와 끄기'
$locked = [System.IO.File]::Open((Join-Path $fixture 'a_report.txt'), 'Open', 'Read', 'None')
press '이름 바꾸기'
$dialog = wait_dialog
$choice = dialog_texts $dialog
capture 'g31' ([IntPtr]$dialog.Current.NativeWindowHandle)
front ([IntPtr]$dialog.Current.NativeWindowHandle)
[System.Windows.Forms.SendKeys]::SendWait('{ESC}')
Start-Sleep -Milliseconds 500
$still_open = (wait_dialog) -ne $null
press_in $dialog '전체 되돌리기'
$reverted = (status_message) -eq '모든 항목을 실행 전 이름으로 되돌렸습니다.'
press '이름 바꾸기'
$dialog = wait_dialog
press_in $dialog '성공한 항목 유지'
$kept_message = status_message
$locked.Close()
$passed = $still_open -and $reverted -and
    ($kept_message -match '^7개 항목의 이름을 바꿨습니다\. 실패한 1개는 그대로 있습니다')
check 'G31' $passed "$($choice -join ' / ') | $kept_message"
