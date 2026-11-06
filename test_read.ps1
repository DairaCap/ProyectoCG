$lines = [System.IO.File]::ReadAllLines('scratch_anteproyecto_clean.txt')
Write-Host "Total lines: $($lines.Length)"
$matched = 0
foreach ($line in $lines) {
    if ($line -match 'casa|interac|proyecto|animaci|ilumina|objetiv|pantalla|modelo|escena|camara|luz|puerta') {
        Write-Host $line
        $matched++
        if ($matched -ge 40) { break }
    }
}
