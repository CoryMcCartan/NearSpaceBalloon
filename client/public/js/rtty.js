/*
 * Parses RTTY string, returning object containing most recent probe data.
 */
function processRTTY(text) {
    // each line is a new broadcast from probe
    let entries = text.split("\n");
    let length = entries.length;

    let data = {
        time: [],
        temp: [],
        pres: [],
        coords: [],
        alt: [],
        batt: [],
    };
    
    for (let entry of entries) {
        // regex to extract numbers
        let matches = entry.match(/GSEA~S(\d+)T(\d+)P(\d+)X(\d+)Y(\d+)A(\d+)B(\d+)~/);        
        // skip invalid entries
        if (matches === null) continue;

        // parse each match as a number, adjusting units & magnitudes as necessary
        // (probe can only send integers, not floating-point numbers)
        let minutes = +matches[1] / 60;
        let kelvin = +matches[2] / 10;
        let pressure = +matches[3];
        let farenheit = (kelvin - 273.15) * 1.8 + 32;
        let longitude = -matches[4] / 10000;
        let latitude = +matches[5] / 10000;
        let altitude = +matches[6] / 10;
        let voltage = +matches[7] / 1000;

        // save data
        data.time.push(minutes);
        data.temp.push(farenheit);
        data.pres.push(pressure);
        data.coords.push({
            lat: latitude,
            lng: longitude,
        });
        data.alt.push(altitude);
        data.batt.push(voltage);
    }

    return data;
}

if (typeof module !== "undefined" && module.exports) 
    module.exports = { processRTTY };
