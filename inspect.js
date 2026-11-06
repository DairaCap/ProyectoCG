const fs = require('fs');

function inspectPdf(filename) {
    const buf = fs.readFileSync(filename);
    console.log(`=== Inspecting ${filename} (${buf.length} bytes) ===`);
    let str = buf.toString('latin1');
    
    // Look for text chunks
    let textMatches = str.match(/\((.*?)\)\s*T[jJ]/g);
    if (textMatches && textMatches.length > 0) {
        console.log(`Found ${textMatches.length} raw text tokens:`);
        let sample = textMatches.map(m => m.replace(/[\(\)]/g, '').replace(/T[jJ]/, '')).join(' ');
        console.log(sample.substring(0, 1500));
    }
}

inspectPdf('Proyecto/EspecificacionesProyectoFinal-LCGEIHC-27-1 (1).pdf');
inspectPdf('Proyecto/Anteproyecto.pdf');
