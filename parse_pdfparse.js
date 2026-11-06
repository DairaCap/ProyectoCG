const fs = require('fs');
const pdf = require('pdf-parse');

async function run() {
    let dataBuffer = fs.readFileSync('Proyecto/Anteproyecto.pdf');
    let data = await pdf(dataBuffer);
    console.log('PDF Pages:', data.numpages);
    console.log('PDF Text length:', data.text.length);
    fs.writeFileSync('anteproyecto_pdfparse.txt', data.text);
    console.log('Sample text:\n', data.text.substring(0, 3000));
}

run().catch(console.error);
