/*
 * NEAR SPACE BALLOON PROJECT
 * FLIGHT COMPUTER CODE
 *
 * GRINNELL SPACE EXPLORATION AGENCY
 *
 * © 2016.  All Rights Reserved.
 */

#include "pressure.h"

SFE_BMP180 pressureSensor;

bool setupPressureSensor() {
    if (pressureSensor.begin()) {
        return true;
    } else {
        return false;
    }
}

float getPressure() {
    char status;
    double temperature, pressure;

    // we have to get a temperature reading first to get a pressure reading.
    status = pressureSensor.startTemperature();
    if (status != 0) {
        delay(status); // wait for measurement to complete

        // we pass in the variable to store temperature in
        status = pressureSensor.getTemperature(temperature); 

        if (status != 0) {
            // ready to start pressure measurement.
            // param is 0-3.  higher means more accuracy, but a longer wait.
            status = pressureSensor.startPressure(2); 
            if (status != 0) {
                delay(status); // wait for the measurement to complete

                // we pass in the pressure variable to store in, and the temperature
                status = pressureSensor.getPressure(pressure, temperature);

                if (status == 0) {
                    pressure = -1; // error value
                }
            }
        } else pressure = -1; // error value
    } else pressure = -1; // error value

    return (float) pressure;
}
