
$lines = Get-Content -Path 'scratch_especificaciones.txt'
$clean = New-Object System.Collections.Generic.List[string]
foreach ($l in $lines) {
    if ($l.Length -gt 15 -and ($l -match '[a-zA-Z]{3,}')) {
        $clean.Add($l)
    }
}
[System.IO.File]::WriteAllLines('scratch_especificaciones_clean.txt', $clean)

$lines2 = Get-Content -Path 'scratch_anteproyecto.txt'
$clean2 = New-Object System.Collections.Generic.List[string]
foreach ($l in $lines2) {
    if ($l.Length -gt 15 -and ($l -match '[a-zA-Z]{3,}')) {
        $clean2.Add($l)
    }
}
[System.IO.File]::WriteAllLines('scratch_anteproyecto_clean.txt', $clean2)
Write-Host "Filtered lines saved."
