start_app
$fixture = make_fixture 'gui_fixture'
pick_path '폴더 안의 파일 추가...' $fixture
direct_edit 3 'direct.txt'
$row = @(row_name (rows)[3])
$cards = @(card_texts)
$passed = ($row -contains 'direct.txt') -and ($row -contains '직접') -and
    ($cards -contains '4번 항목을 "direct.txt"로 직접 수정')
check 'G22' $passed "$($row -join ' / ') | $($cards -join ' | ')"

direct_edit 4 'zzz.txt' '{ESC}'
$row = @(row_name (rows)[4])
check 'G23' ($row -notcontains 'zzz.txt') ($row -join ' / ')

direct_edit 6 'a:b.txt'
$row = @(row_name (rows)[6])
click_row 6
$problem = status_message
capture 'g24'
$passed = ($row -contains '오류') -and ($problem -match '^7번 항목: Windows에서 쓸 수 없는 문자')
check 'G24' $passed "$($row -join ' / ') | $problem"

direct_edit 6 'README'
$row = @(row_name (rows)[6])
click_row 6
$problem = status_message
check 'G25' (($row -contains '충돌') -and ($problem -match '겹칩니다')) "$($row -join ' / ') | $problem"

press '이름 바꾸기'
$refused = status_message
$passed = ($refused -match '^충돌 1개가 있어 이름을 바꾸지 않았습니다') -and
    (Test-Path (Join-Path $fixture 'report.txt'))
check 'G26' $passed "$refused | $(header_text)"
