start_app
$fixture = make_fixture 'gui_fixture'
pick_path '폴더 안의 파일 추가...' $fixture
menu '정렬' '이름순 (문자 순)'
pick_path '파일 추가...' (Join-Path $fixture 'folder_a\inner.txt')
$names = @(current_names)
press '정렬'
$custom = (find_element '사용자 지정') -ne $null
send_keys '{ESC}'
check 'G12' (($names[-1] -eq 'inner.txt') -and $custom) "last=$($names[-1]) custom=$custom"

menu '보기 필터' '변경 (0)'
$all = texts
$guide = ($all -contains '변경 항목이 없습니다') -and ((find_element '전체 보기') -ne $null)
capture 'g13'
press '전체 보기'
check 'G13' ($guide -and ((rows).Count -eq 9)) "guide=$guide rows=$((rows).Count)"

press '수정 추가'
press '바꾸기'
press_text '전부'
type_into_value '여행_'
press '번호'
$all = texts
capture 'g14'
$passed = ($all -contains '시작 번호') -and ($all -contains '여행_001.gz')
check 'G14' $passed 'token settings and example'

press '추가'
$names = @(new_names)
$message = status_message
$passed = ($names[0] -eq '여행_001.gz') -and ($message -eq '수정을 추가했습니다.')
check 'G15' $passed "$($names[0..2] -join ', ') / $message"

press '수정 추가'
press '바꾸기'
press_text '전부'
press '추가'
$message = status_message
capture 'g16'
check 'G16' ($message -eq '적용하지 못했습니다. 새 값을 입력하세요.') $message
press '취소'
