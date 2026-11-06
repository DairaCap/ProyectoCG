const fs = require('fs');
const zlib = require('zlib');

const data = fs.readFileSync('Proyecto/Anteproyecto.pdf');
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
            uncompressed = null;
        }
    }
    if (!uncompressed) continue;
    let text = uncompressed.toString('latin1');
    // Find all strings in parenthesis: (something)
    let parens = text.match(/\(([^()]*)\)/g);
    if (parens && parens.length > 5) {
        let line = parens.map(p => p.slice(1, -1)).join(' ');
        if (line.match(/[a-zA-Z]{3,}/)) {
            out.push(`--- Stream ${idx} ---`);
            out.push(line);
        }
    }
}
fs.writeFileSync('anteproyecto_extracted.txt', out.join('\n'));
console.log('Saved anteproyecto_extracted.txt, lines:', out.length);
