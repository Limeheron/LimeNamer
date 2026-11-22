start_app
capture 'g01'
$all = texts
$passed = ($all -contains '목록이 비어 있습니다') -and ($all -contains '수정이 없습니다')
check 'G01' $passed 'empty guides shown'

$names = @('빼기', '비우기', '실행 취소', '다시 실행', '이름 되돌리기', '이름 바꾸기')
$enabled = @($names | Where-Object { is_enabled $_ })
check 'G02' ($enabled.Count -eq 0) "enabled: $($enabled -join ', ')"

$fixture = make_fixture 'gui_fixture'
pick_path '폴더 안의 파일 추가...' $fixture
$message = status_message
check 'G03' ($message -eq '8개 항목을 추가했습니다.') $message

pick_path '폴더 안의 파일 추가...' $fixture
$message = status_message
$expected = '추가한 항목이 없습니다. 이미 목록에 있는 8개는 건너뛰었습니다.'
check 'G04' ($message -eq $expected) $message
capture 'g04'

pick_path '파일 추가...' (Join-Path $fixture 'folder_a\inner.txt')
$message = status_message
$passed = ($message -eq '1개 항목을 추가했습니다.') -and ((texts) -contains 'inner.txt')
check 'G05' $passed $message
