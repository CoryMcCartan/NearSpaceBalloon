/**
 * GSEA Near-Space Balloon Radio Server
 *
 * © 2017.  All rights reserved.
 */

let path = require("path");
let fs = require("fs");
let xmlrpc = require("xmlrpc");
let express = require("express");
let server = express();
let ejs = require("ejs");

let parser = require("./public/js/rtty.js");

// setup fldigi XML-RPC client
let fldigi = xmlrpc.createClient({
    host: "127.0.0.1",
    port: 7362,
    path: "/RPC2",
});

// store all RTTY data
let rtty_data = "";
rtty_data = fs.readFileSync("./public/assets/test_data.txt", "utf-8");

/* 
 * Fetch new RTTY data from FL-DIGI server
 */
function getRTTY() {
    return new Promise((resolve, reject) => 
        fldigi.methodCall("rx.get_data", [], function(e, value) {
            if (e) return reject(e);

            let new_data = value.toString();
            rtty_data += new_data;

            resolve(new_data);
        })
    );
}

/* 
 * Re-download all RTTY data from FL-DIGI server
 */
function getAllRTTY() {
    return new Promise((resolve, reject) => 
        // figure out how much data there is
        fldigi.methodCall("text.get_rx_length", [], function(e, length) {
            if (e) return reject(e);

            // fetch all of the data
            fldigi.methodCall("text.get_rx", [0, length], function(e, value) {
                if (e) return reject(e);

                rtty_data = value.toString();
                resolve(rtty_data);
            })
        })
    );
}

/*
 * CUSTOM ROUTES
 */

// send new RTTY info
server.get("/rtty/new", function(request, response) {
    getRTTY().then(data => response.send(data))
        .catch(e => response.status(500).send(e));
});

// send all RTTY info
server.get("/rtty/all", function(request, response) {
    if (request.query.force) // re-download everything from scratch.
        getAllRTTY().then(data => response.send(data))
            .catch(e => response.status(500).send(e));
    else // send our cached store of data
        getRTTY().then(() => response.send(rtty_data))
            .catch(e => response.status(500).send(e));
});

// KML data
server.get("/kml/data.kml", function(request, response) {
    let data = parser.processRTTY(rtty_data);  

    let coords = [];
    // add coordinates to geoJSON object
    let length = data.time.length;
    for (let i = 0; i < length; i++) {
        coords.push(`${data.coords[i].lng},${data.coords[i].lat},${data.alt[i]}`);
    }
    let string = coords.join(" ");

    fs.readFile("template.kml", "utf8", (err, template) => {
        let kml = ejs.render(template, {
            coordinates: string,
        });

        response.writeHead(200, {'Content-Type': 'text/xml'});
        response.write(kml);
        response.end();
    })
});
// serve shell KML file
server.use("/kml", express.static(path.join(__dirname, "kml")));


// Static HTTP server
// serve everything in public/
server.use(express.static(path.join(__dirname, "public")));

server.listen(8080, function() {
    console.log("Dashboard started.");
})
