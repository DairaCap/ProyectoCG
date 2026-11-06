
Add-Type -AssemblyName System.IO.Compression.FileSystem

function Extract-PdfText($filePath, $outPath) {
    $bytes = [System.IO.File]::ReadAllBytes($filePath)
    $text = [System.Text.Encoding]::GetEncoding(28591).GetString($bytes)
    $streamRegex = [regex]'stream[\r\n]+(?s)(.*?)[\r\n]+endstream'
    $matches = $streamRegex.Matches($text)
    
    $sb = New-Object System.Text.StringBuilder
    $sb.AppendLine("=== File: $filePath (Streams: $($matches.Count)) ===") | Out-Null

    $i = 0
    foreach ($m in $matches) {
        $i++
        $raw = [System.Text.Encoding]::GetEncoding(28591).GetBytes($m.Groups[1].Value)
        try {
            $ms = New-Object System.IO.MemoryStream(,$raw)
            $b0 = $ms.ReadByte()
            $b1 = $ms.ReadByte()
            $ds = New-Object System.IO.Compression.DeflateStream($ms, [System.IO.Compression.CompressionMode]::Decompress)
            $sr = New-Object System.IO.StreamReader($ds, [System.Text.Encoding]::UTF8)
            $decomp = $sr.ReadToEnd()
            
            # Extract strings in parenthesis
            $strMatches = [regex]::Matches($decomp, '\((.*?)\)')
            $extracted = ($strMatches | ForEach-Object { $_.Groups[1].Value }) -join " "
            if ($extracted.Trim().Length -gt 15) {
                $sb.AppendLine("--- Stream $i ---") | Out-Null
                $sb.AppendLine($extracted) | Out-Null
            }
        } catch {
        }
    }
    [System.IO.File]::WriteAllText($outPath, $sb.ToString(), [System.Text.Encoding]::UTF8)
}

Extract-PdfText 'Proyecto\EspecificacionesProyectoFinal-LCGEIHC-27-1 (1).pdf' 'scratch_especificaciones.txt'
Extract-PdfText 'Proyecto\Anteproyecto.pdf' 'scratch_anteproyecto.txt'
Write-Host "Done extracting PDF texts."
