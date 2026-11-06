$bytes = [System.IO.File]::ReadAllBytes('Proyecto/Proyecto/Casa_coliciones2.fbx')
$s = [System.Text.Encoding]::ASCII.GetString($bytes)
$matches = [regex]::Matches($s, 'Material::([a-zA-Z0-9_\-\.]+)')
$list = @()
foreach ($m in $matches) {
    $list += $m.Groups[1].Value
}
$list | Select-Object -Unique
