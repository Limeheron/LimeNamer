backup_settings
start_app
$fixture = make_fixture 'gui_fixture'
pick_path '폴더 안의 파일 추가...' $fixture
foreach ($column in @('폴더', '생성일', '크기')) {
    if ((headers) -notcontains $column) {
        menu '열' $column
    }
}
$shown = headers
capture 'g06'
$wanted = @('순번', '상태', '현재 이름', '변경 이름', '폴더', '수정일', '생성일', '크기')
$missing = @($wanted | Where-Object { $shown -notcontains $_ })
check 'G06' ($missing.Count -eq 0) ($shown -join ', ')

foreach ($column in @('폴더', '생성일', '크기')) {
    menu '열' $column
}
close_app
start_app
$after = headers
check 'G07' (($after -notcontains '폴더') -and ($after -notcontains '크기')) ($after -join ', ')

pick_path '폴더 안의 파일 추가...' $fixture
select_rows @(0, 1, 2, 4)
$header = header_text
check 'G08' (($header -match '선택 4개') -and (is_enabled '빼기')) $header

(find_all ([System.Windows.Automation.ControlType]::DataGrid))[0].SetFocus()
select_rows @(0, 1, 2, 4)
send_keys '{DEL}'
$removed = status_message
$count_after = (rows).Count
send_keys '^z'
$undone = status_message
$passed = ($count_after -eq 4) -and ($undone -eq '실행 취소했습니다.') -and ((rows).Count -eq 8)
check 'G09' $passed "$removed / $undone"

menu '정렬' '이름순 (자연 정렬)'
$natural = (current_names) -join ' | '
menu '정렬' '이름순 (문자 순)'
$ordinal = (current_names) -join ' | '
$passed = ($natural -match 'photo_1.jpg \| photo_2.jpg \| photo_10.jpg') -and
    ($ordinal -match 'photo_1.jpg \| photo_10.jpg \| photo_2.jpg')
check 'G10' $passed "$natural // $ordinal"
