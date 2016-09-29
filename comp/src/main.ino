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

#define S_SETUP 1
#define S_PRELAUNCH 2
#define S_LOW_ALTITUDE 3
#define S_HIGH_ALTITUDE 4
#define S_FALL 5
#define S_PRELANDING 6
#define S_LANDING 7

#define LOW_HIGH_THRESHOLD 1000

ProbeInfo info;
unsigned long previous = 0;

float prevAltitude;
float maxAltitude = 0;
float basePressure;
byte state = S_SETUP;

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


    switch (state) {
        case S_SETUP:
            prevAltitude = info.altitude;
            basePressure = getPressure();
            state = S_PRELAUNCH;
            break;
        case S_PRELAUNCH:
            // set path  WIDE1-1, WIDE2-2
            if (prevAltitude - info.altitude > 10) // starts climbing
                state = S_LOW_ALTITUDE;
            break;
        case S_LOW_ALTITUDE:
            // set path WIDE2-1
            if (info.altitude > LOW_HIGH_THRESHOLD) 
                state = S_LOW_ALTITUDE;
            break;
        case S_HIGH_ALTITUDE:
            // set path NONE
            if (maxAltitude - info.altitude > 10) {// start falling
                state = S_FALL;
                // broadcast 'FALL' and max altitude
            } else 
                maxAltitude = max(maxAltitude, info.altitude);
            break;
        case S_FALL:
            if (info.altitude < LOW_HIGH_THRESHOLD) 
                state = S_PRELANDING;
            break;
        case S_PRELANDING:
            // set path WIDE2-1
            // broadcast more frequently
            if (abs(prevAltitude - info.altitude) < 4 && info.speed < 1)
                state = S_LANDING;
            else
                prevAltitude = info.altitude;
            break;
        case S_LANDING:
            // set path  WIDE1-1, WIDE2-2
            // stop recording temp and pressure
            break;
        default:
            fatal_error("Not a state");
            break;
    }

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
