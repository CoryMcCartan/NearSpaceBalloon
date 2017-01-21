/*
 * Map-related functions.
 */


let map, path, marker;

/*
 * Set up map for plotting path.  Called automatically.
 */
function initMap() {
    let grinnell = {lat: 41.7491836, lng: -92.7213833};
    map = new google.maps.Map($("#map"), {
        zoom: 8,
        center: grinnell,
        mapTypeId: 'terrain',
        scrollwheel: false,
        streetViewControl: false,
    });
    // map needs to resize after being placed in dynamic card container
    setTimeout(() => google.maps.event.trigger(map, "resize"), 1000);
}

/*
 * Plot the (initial) path of the balloon on the map.
 */
function plotPath(coords) {
    path = new google.maps.Polyline({
        path: coords,
        geodesic: true,
        strokeColor: '#c00000',
        strokeOpacity: 0.8,
        strokeWeight: 4,
    });

    path.setMap(map);

    // add marker at end of path, representing balloon
    var loc = coords.last();
    marker = new google.maps.Marker({
        position: loc,
        //icon: "assets/balloon.png",
        map,
    })

    // center map around current balloon location
    setTimeout(() => map.panTo(loc), 1000);  
}

/*
 * Update the plotted balloon path.
 */
function addPathPoints(coords) {
    let pathObj = path.getPath();
    for (let pt of coords) {
        pathObj.push(new google.maps.LatLng(pt));
    }

    var loc = coords.last();
    marker.setPosition(loc);
    map.panTo(loc);   // re-center map 
}
