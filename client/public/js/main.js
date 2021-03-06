/*
 * Main client script file.
 *
 * Grinnell Space Exploration Agency
 * © 2017. All Rights Reserved.
 */

const TESTING_MODE = true;
let MAP = true;
const POLLING_INTERVAL = 10; // seconds

let tempChart, presChart, altChart, battChart;

/*
 * Program entry point.
 * Fetch initial RTTY data, set up display.
 */
async function main() {
    // get RTTY data from node server
    let response = await fetch("/rtty/all");
    if (!response.ok && !TESTING_MODE) return failConnection();
    if (!response.ok && TESTING_MODE) 
        response = await fetch("assets/test_data.txt");
    let text = await response.text();

    // show cards if successful
    $(".page-content").hidden = false;
    $("#loading").hidden = true;
    if (!navigator.onLine) {
        $(".map-card").hidden = true;
        MAP = false;
    }
    showToast("Connection established.");

    // parse RTTY strings
    let data = processRTTY(text);

    // set initial dashboard numbers
    updateBasicStats(data);

    // draw charts for major stats
    tempChart = drawLineChart("#temp-graph", {
        time: data.time,
        data: data.temp,
        label: "Temperature (°F)",
    });
    presChart = drawLineChart("#pres-graph", {
        time: data.time,
        data: data.pres,
        label: "Pressure (mb)",
    });
    altChart = drawLineChart("#alt-graph", {
        time: data.time,
        data: data.alt,
        label: "Altitude (m)",
    });
    battChart = drawLineChart("#batt-graph", {
        time: data.time,
        data: data.batt,
        label: "Battery Voltage (V)",
        yTick: x => x.toFixed(1),
        yFormat: x => x.toFixed(1) + " V",
    });

    // plot path of balloon on map
    if (MAP) plotPath(data.coords);

    // set up to query new RTTY data every x seconds
    setInterval(update, POLLING_INTERVAL * 1000);
}

/*
 * Fetch new RTTY data and update display.
 */
async function update() {
    // get RTTY data from server
    let response = await fetch("/rtty/new");
    if (!response.ok && !TESTING_MODE) return failConnection();
    if (!response.ok && TESTING_MODE) 
        response = await fetch("assets/test_new_data.txt");
    let text = await response.text();

    // parse RTTY string
    let data = processRTTY(text);
    if (data.time.length === 0) return; // if no data, quit

    // update dashboard numbers
    updateBasicStats(data);

    // update charts
    addData(tempChart, data.time, data.temp);
    addData(presChart, data.time, data.pres);
    addData(altChart, data.time, data.alt);
    addData(battChart, data.time, data.batt);
    
    // update map
    if (MAP) addPathPoints(data.coords);
}


/*
 * Update dashboard numbers with most recent data.
 */
function updateBasicStats(data) {
    $("#temp").innerText = Math.round(data.temp.last()) + " °F";
    $("#pres").innerText = data.pres.last() + " mb";
    $("#alt").innerText = data.alt.last() + " m";
    $("#time").innerText = "T+" + toTimeString(data.time.last());
    $("#batt").innerText = data.batt.last().toFixed(1);
}

/*
 * Convert # of minutes into time string (12:34).
 * Helper function.
 */
function toTimeString(minutes) {
    let hours = Math.floor(minutes / 60);
    minutes = Math.floor(minutes) % 60;

    return ("0"+hours).slice(-2) + ":" + ("0"+minutes).slice(-2);
}

/*
 * If no connection to probe can be established.
 */
function failConnection() {
    setTimeout(() => {
        $("#loading").hidden = true;
        showToast("Failed to establish connection.");
    }, 2000);
}

/*
 * Display a toast (message) at the bottom of the screen.
 * Helper function.
 */
function showToast(msg) {
    $("#toast").MaterialSnackbar.showSnackbar({
        message: msg,
        timeout: 4000,
    });
}

// add function to all array objects to return last element.
Array.prototype.last = function() {
    return this[this.length - 1];
};
