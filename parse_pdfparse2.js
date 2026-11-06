const fs = require('fs');
const { PDFParse } = require('pdf-parse');

async function run() {
    let dataBuffer = fs.readFileSync('Proyecto/Anteproyecto.pdf');
    const parser = new PDFParse(dataBuffer);
    const result = await parser.getText();
    console.log('Result text length:', result.text ? result.text.length : 0);
    fs.writeFileSync('anteproyecto_parsed_full.txt', result.text || JSON.stringify(result));
    console.log((result.text || '').substring(0, 2000));
}

run().catch(async (e) => {
    console.error('Error with getText, inspecting PDFParse methods:');
    try {
        const parser = new PDFParse(fs.readFileSync('Proyecto/Anteproyecto.pdf'));
        console.log(Object.getOwnPropertyNames(Object.getPrototypeOf(parser)));
    } catch(err) {
        console.error(err);
    }
});
