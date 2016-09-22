/*
 * NEAR SPACE BALLOON PROJECT
 * FLIGHT COMPUTER CODE
 *
 * GRINNELL SPACE EXPLORATION AGENCY
 *
 * © 2016.  All Rights Reserved.
 */

#include "temp.h"
#include "pressure.h"
#include "gps.h"

#define INTERVAL 2000

ProbeInfo info;
unsigned long previous = 0;

// runs once at start
void setup() {
    Serial.begin(9600); // begin logging

    setupGPS() || error("GPS sensor setup failed.");
    setupTemperatureSensor() || error("Temperature sensor setup failed.");
    setupPressureSensor() || error("Pressure sensor setup failed.");
}

// runs continuously
void loop() {
    getGPSData(&info);

    unsigned long current = millis();
    if (current - previous >= INTERVAL) {
        previous = current;

        double kelvin = getTemperature();
        double farenheit = toFarenheit(kelvin);
        Serial.print("T = ");
        Serial.print(farenheit);
        Serial.print(" °F  ");

        double mb = getPressure();
        Serial.print("P = ");
        Serial.print(mb);
        Serial.print(" mb  ");

        Serial.print("LAT. = ");
        Serial.print(info.latitude);
        Serial.print("°  LONG. = ");
        Serial.print(info.longitude);
        Serial.print("°  ALT. = ");
        Serial.print(info.altitude);
        Serial.print(" m  SPD. = ");
        Serial.print(info.speed);
        Serial.print(" m/s  (");
        Serial.print(info.satellites);
        Serial.println(" satellites )");
    }
}

void end() { while (true) delay(1000); }

bool error(char * msg) {
    Serial.print("Error: ");
    Serial.println(msg);
    return false;
}

bool fatal_error(char * msg) {
    Serial.print("Fatal Error: ");
    Serial.println(msg);
    
    end();
    return false;
}
