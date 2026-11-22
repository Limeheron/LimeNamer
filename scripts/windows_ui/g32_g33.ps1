start_app
$fixture = make_fixture 'gui_fixture'
pick_path '폴더 안의 파일 추가...' $fixture
add_insert_mod 'a_'
click_named '켜기와 끄기'

function snapshot()
{
    return "$(@(rows).Count):$((@(current_names))[0]):$(@(new_names)[0])"
}

$before = @(snapshot)
menu '정렬' '이름순 (문자 순)'
$before += snapshot
(find_all ([System.Windows.Automation.ControlType]::DataGrid))[0].SetFocus()
select_rows @(0)
send_keys '{DEL}'
$before += snapshot
click_named '켜기와 끄기'
wait_until { @(new_names)[0] -like 'a_*' } 10 | Out-Null
$before += snapshot
$undo = @()
for ($i = 0; $i -lt 3; $i++) {
    send_keys '^z'
    $undo += snapshot
}
$redo = @()
for ($i = 0; $i -lt 3; $i++) {
    send_keys '^y'
    $redo += snapshot
}
$passed = (($undo -join '|') -eq (@($before[2], $before[1], $before[0]) -join '|')) -and
    (($redo -join '|') -eq (@($before[1], $before[2], $before[3]) -join '|'))
check 'G32' $passed "before=$($before -join ' ') undo=$($undo -join ' ') redo=$($redo -join ' ')"

press '수정 추가'
press '바꾸기'
press_text '전부'
type_into_value 'same'
press '추가'
$conflicts = ((texts) -match '^충돌').Count -gt 0
press '중복 자동 조정'
press '켜기'
$names = @(new_names) -join ','
$all_combos = @(combos)
combo_select $all_combos[0] '이름_1'
$underscore = @(new_names) -join ','
combo_select $all_combos[0] '이름-1'
$dash = @(new_names) -join ','
combo_select $all_combos[1] '모두 번호 붙이기'
$number_all = @(new_names) -join ','
capture 'g33'
$passed = $conflicts -and ($names -match 'same \(1\)') -and ($underscore -match 'same_1') -and
    ($dash -match 'same-1') -and ($number_all -match 'same-1\.txt')
check 'G33' $passed "$names // $underscore // $dash // $number_all"
