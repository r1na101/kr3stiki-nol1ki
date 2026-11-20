param
(
    [string]$Path,
    [int]$X = 50,  
    [int]$Y = 50,
    [double]$Scale = 1.0,
    [int]$Ms = 0,
    [int]$ParentPid = 0
)

# координаты и размеры в пикселях экрана
Add-Type -TypeDefinition @"
using System.Runtime.InteropServices;
public class Dpi {
    [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
}
"@
[void][Dpi]::SetProcessDPIAware()

# расширение файла
$ext = [System.IO.Path]::GetExtension($Path).ToLower()

if ($ext -eq '.mp4' -or $ext -eq '.wmv' -or $ext -eq '.avi')
{
    # ---------------- видео ================
    Add-Type -AssemblyName PresentationFramework, PresentationCore, WindowsBase

    # окошко
    $win = New-Object System.Windows.Window
    $win.WindowStyle = 'None'
    $win.ResizeMode = 'NoResize'
    $win.ShowInTaskbar = $false
    $win.Topmost = $true
    $win.ShowActivated = $false
    $win.WindowStartupLocation = 'Manual'
    $win.Background = [System.Windows.Media.Brushes]::Black
    $win.Left = $X / $Scale
    $win.Top = $Y / $Scale
    $win.Width = 384
    $win.Height = 272

    $grid = New-Object System.Windows.Controls.Grid

    $me = New-Object System.Windows.Controls.MediaElement
    $me.LoadedBehavior = [System.Windows.Controls.MediaState]::Manual
    $me.UnloadedBehavior = [System.Windows.Controls.MediaState]::Manual
    $me.Stretch = [System.Windows.Media.Stretch]::Fill
    $me.Source = [System.Uri]::new((Resolve-Path -LiteralPath $Path).Path)
    $me.Add_MediaEnded({ $win.Close() })
    [void]$grid.Children.Add($me)

    # кнопка X для видео
    $btn = New-Object System.Windows.Controls.Button
    $btn.Content = 'X'
    $btn.Width = 30
    $btn.Height = 24
    $btn.FontWeight = 'Bold'
    $btn.HorizontalAlignment = 'Right'
    $btn.VerticalAlignment = 'Top'
    $btn.Background = [System.Windows.Media.Brushes]::Firebrick
    $btn.Foreground = [System.Windows.Media.Brushes]::White
    $btn.BorderThickness = New-Object System.Windows.Thickness(0)
    $btn.Add_Click({ $win.Close() })
    [void]$grid.Children.Add($btn)

    # проверка игры для видео
    $script:elapsed = 0
    $timer = New-Object System.Windows.Threading.DispatcherTimer
    $timer.Interval = [TimeSpan]::FromMilliseconds(500)
    $timer.Add_Tick({
        $script:elapsed += 500
        if ($Ms -gt 0 -and $script:elapsed -ge $Ms) { $win.Close(); return }
        if ($ParentPid -gt 0) {
            try { [void][System.Diagnostics.Process]::GetProcessById($ParentPid) }
            catch { $win.Close() }
        }
    })
    $timer.Start()

    $win.Add_Loaded({ $me.Play() })
    $win.Content = $grid
    [void]$win.ShowDialog()
}
else
{
    # ================ спрайт and GIF ----------------
    Add-Type -AssemblyName System.Windows.Forms, System.Drawing
    Add-Type -ReferencedAssemblies System.Windows.Forms, System.Drawing -TypeDefinition @"
using System.Windows.Forms;
public class SpriteForm : Form {
    protected override bool ShowWithoutActivation { get { return true; } }
    protected override CreateParams CreateParams {
        get { var cp = base.CreateParams; cp.ExStyle |= 0x08000000 | 0x80; return cp; }
    }
}
"@
    $img = [System.Drawing.Image]::FromFile($Path)
    $form = New-Object SpriteForm
    $form.FormBorderStyle = 'None'
    $form.StartPosition = 'Manual'
    $form.Location = New-Object System.Drawing.Point($X, $Y)
    $form.ClientSize = New-Object System.Drawing.Size([int]($img.Width * $Scale), [int]($img.Height * $Scale))
    $form.ShowInTaskbar = $false
    $form.TopMost = $true

    $pb = New-Object System.Windows.Forms.PictureBox
    $pb.Dock = 'Fill'
    $pb.SizeMode = 'StretchImage'
    $pb.Image = $img
    $form.Controls.Add($pb)

    # кнопка X для фото гиф
    $btn = New-Object System.Windows.Forms.Label
    $btn.Text = 'X'
    $btn.Font = New-Object System.Drawing.Font('Segoe UI', 10, [System.Drawing.FontStyle]::Bold)
    $btn.ForeColor = [System.Drawing.Color]::White
    $btn.BackColor = [System.Drawing.Color]::FromArgb(200, 40, 40)
    $btn.TextAlign = 'MiddleCenter'
    $btn.Size = New-Object System.Drawing.Size(30, 24)
    $btn.Location = New-Object System.Drawing.Point(($form.ClientSize.Width - 30), 0)
    $btn.Cursor = [System.Windows.Forms.Cursors]::Hand
    $btn.Add_Click({ $form.Close() })
    $pb.Controls.Add($btn)
    $btn.BringToFront()

    # проверка игры для фото гиф
    $script:elapsed = 0
    $timer = New-Object System.Windows.Forms.Timer
    $timer.Interval = 500
    $timer.Add_Tick({
        $script:elapsed += 500
        if ($Ms -gt 0 -and $script:elapsed -ge $Ms) { $form.Close(); return }
        if ($ParentPid -gt 0) {
            try { [void][System.Diagnostics.Process]::GetProcessById($ParentPid) }
            catch { $form.Close() }
        }
    })
    $timer.Start()

    [System.Windows.Forms.Application]::Run($form)
}