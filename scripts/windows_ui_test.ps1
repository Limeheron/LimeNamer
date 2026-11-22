#!powershell
$WINDOWS_UI_TEST_VERSION = '0.1.0'

$ErrorActionPreference = 'Stop'
$ROOT_DIR = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$WORK_DIR = Join-Path $ROOT_DIR 'temp\ui_test'
$CASE_DIR = Join-Path $PSScriptRoot 'windows_ui'
$script:APP_PATH = Join-Path $ROOT_DIR 'build\dotnet\bin\limenamer_app\debug\LimeNamer.exe'
$LOG_FILE = Join-Path $WORK_DIR 'log.txt'
$DONE_FILE = Join-Path $WORK_DIR 'done.txt'
$CASE_FILE = Join-Path $WORK_DIR 'case.txt'

Add-Type -AssemblyName UIAutomationClient, UIAutomationTypes, System.Windows.Forms, System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;

public static class UI_NATIVE
{
    [StructLayout(LayoutKind.Sequential)]
    public struct RECT
    {
        public int left;
        public int top;
        public int right;
        public int bottom;
    }

    [DllImport("user32.dll")]
    public static extern IntPtr GetForegroundWindow();

    [DllImport("user32.dll")]
    public static extern uint GetWindowThreadProcessId(IntPtr window, out uint process_id);

    [DllImport("user32.dll")]
    public static extern bool SetForegroundWindow(IntPtr window);

    [DllImport("user32.dll")]
    public static extern bool GetWindowRect(IntPtr window, out RECT rect);

    [DllImport("user32.dll")]
    public static extern bool SetProcessDpiAwarenessContext(IntPtr context);

    [DllImport("user32.dll", CharSet = CharSet.Unicode)]
    public static extern IntPtr SendMessage(IntPtr window, uint message, IntPtr w, string l);

    [DllImport("user32.dll")]
    public static extern IntPtr SendMessage(IntPtr window, uint message, IntPtr w, IntPtr l);

    [DllImport("user32.dll")]
    public static extern bool PostMessage(IntPtr window, uint message, IntPtr w, IntPtr l);

    [DllImport("user32.dll")]
    public static extern bool SetCursorPos(int x, int y);

    [DllImport("user32.dll")]
    public static extern void mouse_event(uint flags, uint x, uint y, uint data, IntPtr extra);

    [DllImport("user32.dll")]
    public static extern bool IsWindowVisible(IntPtr window);

    [DllImport("user32.dll")]
    public static extern bool IsIconic(IntPtr window);

    [DllImport("user32.dll")]
    public static extern bool PrintWindow(IntPtr window, IntPtr dc, uint flags);

    [StructLayout(LayoutKind.Sequential)]
    public struct KEYBDINPUT
    {
        public ushort vk;
        public ushort scan;
        public uint flags;
        public uint time;
        public IntPtr extra;
    }

    [StructLayout(LayoutKind.Explicit, Size = 40)]
    public struct INPUT
    {
        [FieldOffset(0)]
        public uint type;
        [FieldOffset(8)]
        public KEYBDINPUT key;
    }

    [DllImport("user32.dll")]
    public static extern uint SendInput(uint count, INPUT[] inputs, int size);

    public static void type_text(string text)
    {
        foreach (char letter in text) {
            INPUT down = new INPUT();
            down.type = 1;
            down.key.scan = letter;
            down.key.flags = 0x0004;
            INPUT up = down;
            up.key.flags = 0x0004 | 0x0002;
            SendInput(2, new INPUT[] { down, up }, Marshal.SizeOf(typeof(INPUT)));
            System.Threading.Thread.Sleep(20);
        }
    }

    [DllImport("dwmapi.dll")]
    public static extern int DwmGetWindowAttribute(IntPtr window, int attribute, out RECT rect,
        int size);
}
'@

[UI_NATIVE]::SetProcessDpiAwarenessContext([IntPtr](-4)) | Out-Null
$AE = [System.Windows.Automation.AutomationElement]
$TREE = [System.Windows.Automation.TreeScope]
$script:app = $null

function write_log([string]$text)
{
    Add-Content -Path $LOG_FILE -Value $text -Encoding UTF8
}

function check([string]$case, [bool]$passed, [string]$detail)
{
    $result = if ($passed) { 'PASS' } else { 'FAIL' }
    write_log "$result $case $detail"
}

function stop_case([string]$reason)
{
    throw "STOP: $reason"
}

function start_app()
{
    $script:app = Start-Process -FilePath $script:APP_PATH -PassThru -WindowStyle Normal
    for ($i = 0; $i -lt 50; $i++) {
        Start-Sleep -Milliseconds 200
        $script:app.Refresh()
        if ($script:app.MainWindowHandle -ne [IntPtr]::Zero) {
            Start-Sleep -Milliseconds 2000
            return
        }
    }
    stop_case 'main window did not appear'
}

function close_app()
{
    if ($script:app -ne $null -and -not $script:app.HasExited) {
        $script:app.CloseMainWindow() | Out-Null
        if (-not $script:app.WaitForExit(5000)) {
            $script:app.Kill()
        }
    }
}

function app_windows()
{
    $condition = New-Object System.Windows.Automation.PropertyCondition(
        $AE::ProcessIdProperty, $script:app.Id)
    return $AE::RootElement.FindAll($TREE::Children, $condition)
}

function main_window()
{
    return $AE::FromHandle($script:app.MainWindowHandle)
}

function front([IntPtr]$handle = [IntPtr]::Zero)
{
    if ($handle -eq [IntPtr]::Zero) {
        $handle = $script:app.MainWindowHandle
    }
    [UI_NATIVE]::SetForegroundWindow($handle) | Out-Null
    Start-Sleep -Milliseconds 200
    $owner = 0
    [UI_NATIVE]::GetWindowThreadProcessId([UI_NATIVE]::GetForegroundWindow(), [ref]$owner) |
        Out-Null
    if ($owner -ne $script:app.Id) {
        stop_case 'LimeNamer is not the foreground window'
    }
}

function find_element([string]$name, $scope = $null)
{
    $by_name = New-Object System.Windows.Automation.PropertyCondition($AE::NameProperty, $name)
    $by_help = New-Object System.Windows.Automation.PropertyCondition(
        $AE::HelpTextProperty, $name)
    $condition = New-Object System.Windows.Automation.OrCondition($by_name, $by_help)
    $roots = if ($scope -ne $null) { @($scope) } else { app_windows }
    foreach ($root in $roots) {
        $found = $root.FindFirst($TREE::Descendants, $condition)
        if ($found -ne $null) {
            return $found
        }
    }
    return $null
}

function press([string]$name)
{
    $element = find_element $name
    if ($element -eq $null) {
        stop_case "element not found: $name"
    }
    front
    $pattern = $null
    if ($element.TryGetCurrentPattern(
            [System.Windows.Automation.InvokePattern]::Pattern, [ref]$pattern)) {
        $pattern.Invoke()
    } elseif ($element.TryGetCurrentPattern(
            [System.Windows.Automation.TogglePattern]::Pattern, [ref]$pattern)) {
        $pattern.Toggle()
    } elseif ($element.TryGetCurrentPattern(
            [System.Windows.Automation.SelectionItemPattern]::Pattern, [ref]$pattern)) {
        $pattern.Select()
    } else {
        stop_case "element cannot be pressed: $name"
    }
    Start-Sleep -Milliseconds 400
}

function is_enabled([string]$name)
{
    $element = find_element $name
    if ($element -eq $null) {
        return $null
    }
    return $element.Current.IsEnabled
}

function send_keys([string]$keys)
{
    front
    [System.Windows.Forms.SendKeys]::SendWait($keys)
    Start-Sleep -Milliseconds 400
}

function texts()
{
    $condition = New-Object System.Windows.Automation.PropertyCondition(
        $AE::ControlTypeProperty, [System.Windows.Automation.ControlType]::Text)
    $result = @()
    foreach ($element in (main_window).FindAll($TREE::Descendants, $condition)) {
        $result += $element.Current.Name
    }
    return $result
}

function status_message()
{
    $all = texts
    for ($i = $all.Count - 2; $i -ge 0; $i--) {
        if ($all[$i] -match '^\d\d:\d\d$') {
            return $all[$i + 1]
        }
    }
    return ''
}

function capture([string]$name, [IntPtr]$handle = [IntPtr]::Zero)
{
    if ($handle -eq [IntPtr]::Zero) {
        $handle = $script:app.MainWindowHandle
    }
    front $handle
    $rect = New-Object UI_NATIVE+RECT
    [UI_NATIVE]::DwmGetWindowAttribute($handle, 9, [ref]$rect, 16) | Out-Null
    $width = $rect.right - $rect.left
    $height = $rect.bottom - $rect.top
    $bitmap = New-Object System.Drawing.Bitmap($width, $height)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    $dc = $graphics.GetHdc()
    [UI_NATIVE]::PrintWindow($handle, $dc, 2) | Out-Null
    $graphics.ReleaseHdc($dc)
    $bitmap.Save((Join-Path $WORK_DIR "$name.png"), [System.Drawing.Imaging.ImageFormat]::Png)
    $graphics.Dispose()
    $bitmap.Dispose()
}

function make_fixture([string]$name)
{
    $folder = Join-Path $WORK_DIR $name
    if (Test-Path $folder) {
        Remove-Item -Recurse -Force $folder
    }
    New-Item -ItemType Directory -Force (Join-Path $folder 'folder_a') | Out-Null
    $files = @('photo_1.jpg', 'photo_2.jpg', 'photo_10.jpg', 'report.txt', 'old_report.txt',
        'README', 'archive.tar.gz', '한글 문서.txt', 'folder_a\inner.txt')
    foreach ($file in $files) {
        New-Item -ItemType File -Force (Join-Path $folder $file) | Out-Null
    }
    return $folder
}

function pick_path([string]$menu_name, [string]$path)
{
    press '추가 메뉴'
    press $menu_name
    $window_type = New-Object System.Windows.Automation.PropertyCondition(
        $AE::ControlTypeProperty, [System.Windows.Automation.ControlType]::Window)
    $dialog = $null
    for ($i = 0; $i -lt 25 -and $dialog -eq $null; $i++) {
        Start-Sleep -Milliseconds 200
        $dialog = (main_window).FindFirst($TREE::Descendants, $window_type)
    }
    if ($dialog -eq $null) {
        stop_case 'folder dialog did not appear'
    }
    $handle = [IntPtr]$dialog.Current.NativeWindowHandle
    front $handle
    $name_box = $null
    $button = $null
    $all = [System.Windows.Automation.Condition]::TrueCondition
    foreach ($element in $dialog.FindAll($TREE::Descendants, $all)) {
        $id = $element.Current.AutomationId
        if (($id -eq '1152' -or $id -eq '1148') -and $element.Current.ClassName -eq 'Edit') {
            $name_box = [IntPtr]$element.Current.NativeWindowHandle
        } elseif ($id -eq '1' -and $element.Current.ClassName -eq 'Button') {
            $button = [IntPtr]$element.Current.NativeWindowHandle
        }
    }
    if ($name_box -eq $null -or $button -eq $null) {
        stop_case 'folder dialog controls not found'
    }
    [UI_NATIVE]::SendMessage($name_box, 0x000C, [IntPtr]::Zero, $path) | Out-Null
    front $handle
    [UI_NATIVE]::SendMessage($button, 0x00F5, [IntPtr]::Zero, [IntPtr]::Zero) | Out-Null
    Start-Sleep -Milliseconds 1500
}

$SETTINGS_FILE = Join-Path $env:LOCALAPPDATA 'LimeNamer\settings.json'
$SETTINGS_BACKUP = Join-Path $WORK_DIR 'settings_backup.json'

function backup_settings()
{
    if ((Test-Path $SETTINGS_FILE) -and -not (Test-Path $SETTINGS_BACKUP)) {
        Copy-Item $SETTINGS_FILE $SETTINGS_BACKUP
    }
}

function restore_settings()
{
    if (Test-Path $SETTINGS_BACKUP) {
        Copy-Item -Force $SETTINGS_BACKUP $SETTINGS_FILE
        Remove-Item -Force $SETTINGS_BACKUP
    }
}

function find_all([System.Windows.Automation.ControlType]$type)
{
    $condition = New-Object System.Windows.Automation.PropertyCondition(
        $AE::ControlTypeProperty, $type)
    return (main_window).FindAll($TREE::Descendants, $condition)
}

function headers()
{
    $result = @()
    foreach ($element in (find_all ([System.Windows.Automation.ControlType]::HeaderItem))) {
        if ($element.Current.Name -ne '') {
            $result += $element.Current.Name
        }
    }
    return $result
}

function rows()
{
    return find_all ([System.Windows.Automation.ControlType]::DataItem)
}

function row_name($row)
{
    $text_type = New-Object System.Windows.Automation.PropertyCondition(
        $AE::ControlTypeProperty, [System.Windows.Automation.ControlType]::Text)
    $names = @()
    foreach ($text in $row.FindAll($TREE::Descendants, $text_type)) {
        $names += $text.Current.Name
    }
    return $names
}

function current_names()
{
    $result = @()
    foreach ($row in (rows)) {
        $names = @(row_name $row)
        $status = [Array]::FindIndex([string[]]$names,
            [Predicate[string]]{ param($text) $text -in @('정상', '변경 없음', '경고', '오류', '충돌') })
        if ($status -ge 0 -and $status + 1 -lt $names.Count) {
            $result += $names[$status + 1]
        }
    }
    return $result
}

function select_rows([int[]]$indexes)
{
    $all = rows
    $first = $true
    foreach ($index in $indexes) {
        $pattern = $all[$index].GetCurrentPattern(
            [System.Windows.Automation.SelectionItemPattern]::Pattern)
        if ($first) {
            $pattern.Select()
            $first = $false
        } else {
            $pattern.AddToSelection()
        }
    }
    Start-Sleep -Milliseconds 300
}

function menu([string]$button, [string]$item)
{
    press $button
    press $item
}

function header_text()
{
    foreach ($text in (texts)) {
        if ($text -match '^\d+개') {
            return $text
        }
    }
    return ''
}

function press_text([string]$label)
{
    $element = find_element $label (main_window)
    if ($element -eq $null) {
        stop_case "text not found: $label"
    }
    $walker = [System.Windows.Automation.TreeWalker]::ControlViewWalker
    $patterns = @([System.Windows.Automation.InvokePattern]::Pattern,
        [System.Windows.Automation.SelectionItemPattern]::Pattern,
        [System.Windows.Automation.TogglePattern]::Pattern)
    while ($element -ne $null) {
        foreach ($pattern_id in $patterns) {
            $pattern = $null
            if ($element.TryGetCurrentPattern($pattern_id, [ref]$pattern)) {
                front
                if ($pattern_id -eq [System.Windows.Automation.InvokePattern]::Pattern) {
                    $pattern.Invoke()
                } elseif ($pattern_id -eq [System.Windows.Automation.TogglePattern]::Pattern) {
                    $pattern.Toggle()
                } else {
                    $pattern.Select()
                }
                Start-Sleep -Milliseconds 400
                return
            }
        }
        $element = $walker.GetParent($element)
    }
    stop_case "no pressable parent: $label"
}

function type_into_value([string]$text)
{
    $document = (find_all ([System.Windows.Automation.ControlType]::Document))[0]
    if ($document -eq $null) {
        stop_case 'value box not found'
    }
    $document.SetFocus()
    front
    [UI_NATIVE]::type_text($text)
    Start-Sleep -Milliseconds 400
}

function new_names()
{
    $result = @()
    foreach ($row in (rows)) {
        $names = @(row_name $row)
        $status = [Array]::FindIndex([string[]]$names,
            [Predicate[string]]{ param($text) $text -in @('정상', '변경 없음', '경고', '오류', '충돌') })
        if ($status -ge 0 -and $status + 2 -lt $names.Count) {
            $result += $names[$status + 2]
        }
    }
    return $result
}

function center_of($element)
{
    $rect = $element.Current.BoundingRectangle
    return @([int]($rect.X + $rect.Width / 2), [int]($rect.Y + $rect.Height / 2))
}

function mouse_move([int]$x, [int]$y)
{
    [UI_NATIVE]::SetCursorPos($x, $y) | Out-Null
    Start-Sleep -Milliseconds 150
}

function mouse_click([int]$x, [int]$y)
{
    front
    mouse_move $x $y
    [UI_NATIVE]::mouse_event(0x0002, 0, 0, 0, [IntPtr]::Zero)
    [UI_NATIVE]::mouse_event(0x0004, 0, 0, 0, [IntPtr]::Zero)
    Start-Sleep -Milliseconds 500
}

function mouse_drag([int]$x1, [int]$y1, [int]$x2, [int]$y2)
{
    front
    mouse_move $x1 $y1
    [UI_NATIVE]::mouse_event(0x0002, 0, 0, 0, [IntPtr]::Zero)
    for ($step = 1; $step -le 20; $step++) {
        mouse_move ([int]($x1 + ($x2 - $x1) * $step / 20)) ([int]($y1 + ($y2 - $y1) * $step / 20))
    }
    Start-Sleep -Milliseconds 200
    [UI_NATIVE]::mouse_event(0x0004, 0, 0, 0, [IntPtr]::Zero)
    Start-Sleep -Milliseconds 600
}

function find_all_named([string]$name)
{
    $by_name = New-Object System.Windows.Automation.PropertyCondition($AE::NameProperty, $name)
    $by_help = New-Object System.Windows.Automation.PropertyCondition(
        $AE::HelpTextProperty, $name)
    $condition = New-Object System.Windows.Automation.OrCondition($by_name, $by_help)
    return (main_window).FindAll($TREE::Descendants, $condition)
}

function card_texts()
{
    $result = @()
    foreach ($text in (texts)) {
        if ($text -match '"' -and $text -match '^(이름|확장자|\d+번)') {
            $result += $text
        }
    }
    return $result
}

function click_named([string]$name, [int]$index = 0)
{
    $all = @(find_all_named $name | Where-Object { -not $_.Current.IsOffscreen })
    if ($all.Count -le $index) {
        stop_case "element not found: $name"
    }
    $at = center_of $all[$index]
    mouse_click $at[0] $at[1]
}

function direct_edit([int]$index, [string]$name, [string]$finish = '{ENTER}')
{
    (find_all ([System.Windows.Automation.ControlType]::DataGrid))[0].SetFocus()
    select_rows @($index)
    send_keys '{F2}'
    $box = $AE::FocusedElement
    if ($box.Current.ControlType -ne [System.Windows.Automation.ControlType]::Edit) {
        stop_case 'direct edit box not focused'
    }
    $box.GetCurrentPattern([System.Windows.Automation.ValuePattern]::Pattern).SetValue($name)
    send_keys $finish
}

function click_row([int]$index)
{
    $at = center_of (rows)[$index]
    mouse_click $at[0] $at[1]
}

function wait_dialog()
{
    $window_type = New-Object System.Windows.Automation.PropertyCondition(
        $AE::ControlTypeProperty, [System.Windows.Automation.ControlType]::Window)
    for ($i = 0; $i -lt 25; $i++) {
        $found = (main_window).FindFirst($TREE::Descendants, $window_type)
        if ($found -eq $null) {
            foreach ($window in (app_windows)) {
                if ($window.Current.NativeWindowHandle -ne [int]$script:app.MainWindowHandle) {
                    $found = $window
                }
            }
        }
        if ($found -ne $null) {
            return $found
        }
        Start-Sleep -Milliseconds 200
    }
    stop_case 'dialog did not appear'
}

function dialog_texts($dialog)
{
    $text_type = New-Object System.Windows.Automation.PropertyCondition(
        $AE::ControlTypeProperty, [System.Windows.Automation.ControlType]::Text)
    $result = @()
    foreach ($text in $dialog.FindAll($TREE::Descendants, $text_type)) {
        $result += $text.Current.Name
    }
    return $result
}

function press_in($dialog, [string]$name)
{
    $element = find_element $name $dialog
    if ($element -eq $null) {
        stop_case "dialog element not found: $name"
    }
    front ([IntPtr]$dialog.Current.NativeWindowHandle)
    $element.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
    Start-Sleep -Milliseconds 800
}

function add_insert_mod([string]$text)
{
    press '수정 추가'
    press '넣기'
    press_text '맨 앞'
    type_into_value $text
    press '추가'
}

function combo_select($combo, [string]$label)
{
    $expand = $combo.GetCurrentPattern([System.Windows.Automation.ExpandCollapsePattern]::Pattern)
    $expand.Expand()
    Start-Sleep -Milliseconds 400
    $item = find_element $label $combo
    $walker = [System.Windows.Automation.TreeWalker]::ControlViewWalker
    $pattern = $null
    while ($item -ne $null -and -not $item.TryGetCurrentPattern(
            [System.Windows.Automation.SelectionItemPattern]::Pattern, [ref]$pattern)) {
        $item = $walker.GetParent($item)
    }
    if ($item -eq $null) {
        stop_case "combo item not found: $label"
    }
    $pattern.Select()
    $expand.Collapse()
    Start-Sleep -Milliseconds 500
}

function combos()
{
    $type = New-Object System.Windows.Automation.PropertyCondition(
        $AE::ControlTypeProperty, [System.Windows.Automation.ControlType]::ComboBox)
    $result = @()
    foreach ($window in (app_windows)) {
        foreach ($combo in $window.FindAll($TREE::Descendants, $type)) {
            $result += $combo
        }
    }
    return $result
}

function window_bitmap([IntPtr]$handle)
{
    $rect = New-Object UI_NATIVE+RECT
    [UI_NATIVE]::DwmGetWindowAttribute($handle, 9, [ref]$rect, 16) | Out-Null
    $width = $rect.right - $rect.left
    $height = $rect.bottom - $rect.top
    $bitmap = New-Object System.Drawing.Bitmap($width, $height)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    $dc = $graphics.GetHdc()
    [UI_NATIVE]::PrintWindow($handle, $dc, 2) | Out-Null
    $graphics.ReleaseHdc($dc)
    $graphics.Dispose()
    return $bitmap
}

function wait_for_display()
{
    for ($i = 0; $i -lt 60; $i++) {
        $bitmap = window_bitmap $script:app.MainWindowHandle
        $x = [int]($bitmap.Width / 2)
        $y = [int]($bitmap.Height / 2)
        $pixel = $bitmap.GetPixel($x, $y)
        $bitmap.Dispose()
        if (-not ($pixel.R -eq 255 -and $pixel.G -eq 255 -and $pixel.B -eq 255)) {
            write_log "display ready after $($i * 2)s"
            return
        }
        Start-Sleep -Seconds 2
    }
    stop_case 'window content was not drawn (is the monitor showing Windows?)'
}

function wait_until([scriptblock]$condition, [int]$seconds)
{
    $watch = [System.Diagnostics.Stopwatch]::StartNew()
    $longest = 0
    while ($watch.Elapsed.TotalSeconds -lt $seconds) {
        $call = [System.Diagnostics.Stopwatch]::StartNew()
        $done = & $condition
        $longest = [Math]::Max($longest, $call.Elapsed.TotalSeconds)
        if ($done) {
            return @([Math]::Round($watch.Elapsed.TotalSeconds, 2), [Math]::Round($longest, 2))
        }
        Start-Sleep -Milliseconds 100
    }
    return @(-1, [Math]::Round($longest, 2))
}

function memory_mb()
{
    $script:app.Refresh()
    return [Math]::Round($script:app.PrivateMemorySize64 / 1MB)
}

function make_many([string]$name, [int]$count)
{
    $folder = Join-Path $WORK_DIR $name
    if (-not (Test-Path $folder)) {
        New-Item -ItemType Directory $folder | Out-Null
        for ($i = 0; $i -lt $count; $i++) {
            [System.IO.File]::Create((Join-Path $folder ('IMG_{0:D6}.JPG' -f $i))).Close()
        }
    }
    return $folder
}

function open_folder_dialog([string]$path)
{
    press '추가 메뉴'
    press '폴더 안의 파일 추가...'
    $window_type = New-Object System.Windows.Automation.PropertyCondition(
        $AE::ControlTypeProperty, [System.Windows.Automation.ControlType]::Window)
    $dialog = $null
    for ($i = 0; $i -lt 25 -and $dialog -eq $null; $i++) {
        Start-Sleep -Milliseconds 200
        $dialog = (main_window).FindFirst($TREE::Descendants, $window_type)
    }
    $name_box = $null
    $button = $null
    foreach ($element in $dialog.FindAll($TREE::Descendants,
            [System.Windows.Automation.Condition]::TrueCondition)) {
        $id = $element.Current.AutomationId
        if ($id -eq '1152' -and $element.Current.ClassName -eq 'Edit') {
            $name_box = [IntPtr]$element.Current.NativeWindowHandle
        } elseif ($id -eq '1' -and $element.Current.ClassName -eq 'Button') {
            $button = [IntPtr]$element.Current.NativeWindowHandle
        }
    }
    [UI_NATIVE]::SendMessage($name_box, 0x000C, [IntPtr]::Zero, $path) | Out-Null
    return $button
}

function usage()
{
    write_log 'usage: write a case name (letters, digits, _) to temp\ui_test\case.txt'
}

New-Item -ItemType Directory -Force $WORK_DIR | Out-Null
Remove-Item -Force -ErrorAction SilentlyContinue $LOG_FILE, $DONE_FILE
try {
    $case = if (Test-Path $CASE_FILE) { (Get-Content $CASE_FILE -Raw).Trim() } else { '' }
    $case_path = Join-Path $CASE_DIR "$case.ps1"
    if ($case -notmatch '^[a-z0-9_]+$' -or -not (Test-Path $case_path)) {
        usage
    } else {
        write_log "case $case"
        . $case_path
    }
} catch {
    write_log "ERROR $($_.Exception.Message)"
} finally {
    close_app
    restore_settings
    Set-Content -Path $DONE_FILE -Value 'done' -Encoding UTF8
}
