start_app
$fixture = make_fixture 'gui_fixture'
pick_path '폴더 안의 파일 추가...' $fixture
press '수정 추가'
press '바꾸기'
press_text '전부'
type_into_value '여행_'
press '번호'
press '추가'

click_named '켜기와 끄기'
$off = @(new_names)[0]
click_named '켜기와 끄기'
$on = @(new_names)[0]
$passed = ($off -eq 'archive.tar.gz') -and ($on -eq '여행_001.gz')
check 'G17' $passed "$off / $on / $(status_message)"

add_insert_mod 'x_'
$added = @(new_names)[0]
$cards = @(card_texts)
$second = find_element $cards[1]
$first = find_element $cards[0]
$from = center_of $second
mouse_move $from[0] $from[1]
$second_y = $second.Current.BoundingRectangle.Y
$grip = @(find_all_named '끌어서 순서를 바꿉니다.' | Where-Object { -not $_.Current.IsOffscreen } |
    Sort-Object { [math]::Abs($_.Current.BoundingRectangle.Y - $second_y) })[0]
$grip_at = center_of $grip
mouse_drag $grip_at[0] $grip_at[1] $grip_at[0] ([int]($first.Current.BoundingRectangle.Y + 2))
$order = @(card_texts)
capture 'g18'
$passed = ($added -eq 'x_여행_001.gz') -and ($order[0] -match 'x_') -and
    (@(new_names)[0] -eq '여행_001.gz')
check 'G18' $passed "$added / $($order -join ' | ') / $(status_message)"

$remove = @(find_all_named '지우기' |
    Where-Object { $_.Current.ControlType -eq [System.Windows.Automation.ControlType]::Button })[0]
$remove_at = center_of $remove
mouse_move $remove_at[0] $remove_at[1]
$remove.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
Start-Sleep -Milliseconds 500
$after_remove = @(card_texts).Count
$removed_message = status_message
send_keys '^z'
$passed = ($after_remove -eq 1) -and (@(card_texts).Count -eq 2)
check 'G19' $passed "$after_remove / $removed_message / $(status_message)"

$card = find_element (@(card_texts)[0])
$at = center_of $card
mouse_click $at[0] $at[1]
press '지정한 항목'
$scope_y = (find_element '지정한 항목').Current.BoundingRectangle.Y
$box = (find_all ([System.Windows.Automation.ControlType]::Edit)) |
    Where-Object { $_.Current.BoundingRectangle.Y -gt $scope_y } |
    Sort-Object { $_.Current.BoundingRectangle.Y } | Select-Object -First 1
if ($box -eq $null) {
    stop_case 'number box not found'
}
$value = $box.GetCurrentPattern([System.Windows.Automation.ValuePattern]::Pattern)
$value.SetValue('1-3')
Start-Sleep -Milliseconds 500
$checked = header_text
capture 'g20'
$value.SetValue('20')
Start-Sleep -Milliseconds 400
press '적용'
$rejected = status_message
$passed = ($checked -match '체크 3개') -and
    ($rejected -eq '적용하지 못했습니다. 번호는 1부터 8까지만 쓸 수 있습니다.')
check 'G20' $passed "$checked / $rejected"

press '실행 취소'
$undo = status_message
press '이름 바꾸기'
$rename = status_message
press '수정 패널'
$panel = status_message
$passed = ($undo -match '^편집 중에는 실행 취소') -and ($rename -match '^편집 중에는 이름을 바꿀') -and
    ($panel -match '^편집 중에는 수정 패널')
check 'G21' $passed "$undo / $rename / $panel"
press '취소'
