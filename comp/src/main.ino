/*
 * NEAR SPACE BALLOON PROJECT
 * MAIN FLIGHT COMPUTER CODE
 *
 * GRINNELL SPACE EXPLORATION AGENCY
 *
 * © 2017.  All Rights Reserved.
 */

#include "temp.h"
#include "pressure.h"
#include "radio.h"
#include "gps.h"
#include "battery.h"
#include "aprs.h"
//#include "rtty.h"

#define INTERVAL 10000 // how often to send radio signals

ProbeInfo info;
unsigned long previous = 0;

// runs once at start
void setup() {
    setupRadio();
    setupGPS();
    setupTemperatureSensor();
    setupPressureSensor();
    ptt(true);
}

//void loop() {
//    send_packet(getTemperature(), getPressure());
//    delay(1000);
//}

// runs continuously
void loop() {
    getGPSData(&info);

    unsigned long current = millis();

    // every INTERVAL ms, aggregate data and broadcast via radio
    if (current - previous >= INTERVAL) {
        previous = current;

        char msg[80];

        // sprintf can only print integers, so we scale data appropriately
        int temp = (int) (10 * getTemperature()); int pres = (int) getPressure();
        long lat = (long) (10000 * info.latitude);
        long lon = (long) (10000 * info.longitude);
        int alt = (int) (10 * info.altitude);
        long bat = getVoltage();

        // create a string to send
        sprintf(msg, "KG5LCD~S%ldT%dP%dX%ldY%ldA%dB%ld~\n", 
                millis()/1000, 
                temp, 
                pres,
                lon,
                lat,
                alt,
                bat
            );

        send_rtty_string(msg);
    }
}

void end() { while (true) delay(1000); }
