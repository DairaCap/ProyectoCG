const fs = require('fs');
const zlib = require('zlib');

function extractPdf(filename, outFile) {
    const data = fs.readFileSync(filename);
    let str = data.toString('latin1');
    const regex = /stream\r?\n([\s\S]*?)\r?\nendstream/g;
    let match;
    let out = [];
    let idx = 0;
    while ((match = regex.exec(str)) !== null) {
        idx++;
        let raw = Buffer.from(match[1], 'latin1');
        let uncompressed;
        try {
            uncompressed = zlib.inflateSync(raw);
        } catch (e) {
            try {
                uncompressed = zlib.inflateRawSync(raw);
            } catch (e2) {
                uncompressed = raw;
            }
        }
        let text = uncompressed.toString('utf8');
        // Let's find Tj, TJ, or parenthesis/bracket strings
        let tjMatches = text.match(/\((.*?)\)\s*T[jJ]/g);
        let TJMatches = text.match(/\[(.*?)\]\s*TJ/g);
        if (tjMatches) {
            out.push(`--- Stream ${idx} (Tj) ---`);
            out.push(tjMatches.map(m => m.replace(/^[\(\[]|[\)\]]\s*T[jJ]/g, '')).join(' '));
        }
        if (TJMatches) {
            out.push(`--- Stream ${idx} (TJ) ---`);
            TJMatches.forEach(tj => {
                let innerMatches = tj.match(/\((.*?)\)/g);
                if (innerMatches) {
                    out.push(innerMatches.map(m => m.replace(/[\(\)]/g, '')).join(''));
                }
            });
        }
    }
    fs.writeFileSync(outFile, out.join('\n'));
    console.log(`Saved ${outFile}, length: ${out.length} lines`);
}

extractPdf('Proyecto/EspecificacionesProyectoFinal-LCGEIHC-27-1 (1).pdf', 'parsed_especificaciones.txt');
extractPdf('Proyecto/Anteproyecto.pdf', 'parsed_anteproyecto.txt');
