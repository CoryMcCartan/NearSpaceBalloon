/*
 * NEAR SPACE BALLOON PROJECT
 * FLIGHT COMPUTER CODE
 *
 * GRINNELL SPACE EXPLORATION AGENCY
 *
 * © 2015.  All Rights Reserved.
 */

#include "temp.h"
#include "pressure.h"
#include "gps.h"

/*
// runs once at start
void setup() {
    Serial.begin(9600); // begin logging

    setupTemperatureSensor() || error("Temperature sensor setup failed.");
    setupPressureSensor() || error("Pressure sensor setup failed.");
}

// runs continuously
void loop() {
    for (int i = 0; i < 15; i++) { // print temp every two seconds for 30 seconds, then end
        double kelvin = getTemperature();
        double farenheit = toFarenheit(kelvin);
        Serial.print("T = ");
        Serial.print(farenheit);
        Serial.print(" °F\t\t");

        double mb = getPressure();
        Serial.print("P = ");
        Serial.print(mb);
        Serial.println(" mb");

        delay(2000);
    }

    end();
}
*/

void setup() {
    setupGPS();
}

void loop() {
    Location position = getGPSData();
    Serial.print("LAT. = ");
    Serial.print(position.latitude / 3e5);
    Serial.print(" LONG. = ");
    Serial.print(position.longitude / 3e5);
    Serial.print(" ALT. = ");
    Serial.print(position.altitude);
    Serial.print("\r");
    delay(1000);
}

void end() { while (true) delay(1000); }

bool error(char *msg) {
    Serial.print("Error: ");
    Serial.println(msg);

    end();
    return false;
}
